#include "Runner.hpp"
#include "AppState.hpp"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string>
#include <sys/wait.h>
#include <thread>

void addLine(SharedLog &sharedLog, const std::string &line) {
  std::lock_guard<std::mutex> lock(sharedLog.mutex);
  sharedLog.lines.push_back(line);
}

void runAnAction(SharedLog &sharedLog, const std::string &command) {
  FILE *pipe = popen((command + " 2>&1").c_str(), "r");
  if (pipe == nullptr) {
    addLine(sharedLog, "err, pipe is nullptr. command did not start");
    return;
  }

  char buffer[512];
  while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    std::string line = buffer;
    if (!line.empty() && line.back() == '\n') {
      line.pop_back();
    }
    addLine(sharedLog, line);
  }

  int status = pclose(pipe);
  int exitcode = -1;
  if (WIFEXITED(status)) {
    exitcode = WEXITSTATUS(status);
  }
  addLine(sharedLog, "finished! exit code " + std::to_string(exitcode));
  addLine(sharedLog, "");
}

void selectAnAction(SharedLog &sharedLog, const std::vector<Action> &actions,
                    int selAction, std::thread &worker,
                    std::atomic<bool> &busy) {
  if (actions.at(selAction).label == "stop") {
    if (!busy) {
      addLine(sharedLog, "nothing to stop.");
      return;
    }
    int result = std::system("pkill -f \"mpremote run\" > /dev/null 2>&1");
    if (result == 0) {
      addLine(sharedLog, "stopped.");
    } else {
      addLine(sharedLog, "nothing stopped, no running mpremote run found");
    }
    return;
  }
  if (busy) {
    addLine(sharedLog, "busy, wait for the current command");
    return;
  }
  std::string command = actions.at(selAction).command;
  addLine(sharedLog, "Selected: " + command);
  if (worker.joinable()) {
    worker.join();
  }
  busy = true;
  worker = std::thread([&sharedLog, &busy, command]() {
    runAnAction(sharedLog, command);
    busy = false;
  });
}
