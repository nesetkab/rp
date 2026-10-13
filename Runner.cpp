#include "Runner.hpp"
#include "AppState.hpp"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <mutex>
#include <string>
#include <sys/wait.h>
#include <thread>

void addLine(SharedState &sharedState, const std::string &line) {
  std::lock_guard<std::mutex> lock(sharedState.mutex);
  sharedState.lines.push_back(line);
}

bool runAnAction(SharedState &sharedState, const std::string &command) {
  FILE *pipe = popen((command + " 2>&1").c_str(), "r");
  if (pipe == nullptr) {
    addLine(sharedState, "err, pipe is nullptr. command did not start");
    return false;
  }

  char buffer[512];
  while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    std::string line = buffer;
    if (!line.empty() && line.back() == '\n') {
      line.pop_back();
    }
    addLine(sharedState, line);
  }

  int status = pclose(pipe);
  int exitcode = -1;
  if (WIFEXITED(status)) {
    exitcode = WEXITSTATUS(status);
  }
  addLine(sharedState, "finished! exit code " + std::to_string(exitcode));
  addLine(sharedState, " ");

  if (exitcode == 0) {
    return true;
  }
  return false;
}

void selectAnAction(SharedState &sharedState,
                    const std::vector<Action> &actions, int selAction,
                    std::thread &worker) {
  if (actions.at(selAction).label == "stop") {
    if (!sharedState.busy) {
      addLine(sharedState, "nothing to stop.");
      return;
    }
    int result = std::system("pkill -f \"mpremote run\" > /dev/null 2>&1");
    if (result == 0) {
      addLine(sharedState, "stopped.");
    } else {
      addLine(sharedState, "nothing stopped, no running mpremote run found");
    }
    return;
  }
  if (sharedState.busy) {
    addLine(sharedState, "busy, wait for the current command");
    return;
  }
  std::string command = actions.at(selAction).command;
  std::string label = actions.at(selAction).label;

  addLine(sharedState, "Selected: " + command);
  if (worker.joinable()) {
    worker.join();
  }

  std::filesystem::file_time_type deployTime =
      std::filesystem::file_time_type::clock::now();
  sharedState.busy = true;

  worker = std::thread([&sharedState, command, label, deployTime]() {
    bool resultPassed = runAnAction(sharedState, command);
    std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &local);
    std::string resultTime = buffer;

    {
      std::lock_guard<std::mutex> lock(sharedState.mutex);
      sharedState.result.resultPassed = resultPassed;
      if (label == "deploy" && resultPassed) {
        sharedState.lastDeploy = deployTime;
      }
      sharedState.result.resultLabel = label;
      sharedState.result.resultTime = resultTime;
    }
  });

  sharedState.busy = false;
}
