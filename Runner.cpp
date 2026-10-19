#include "Runner.hpp"
#include "AppState.hpp"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <mutex>
#include <ncurses.h>
#include <string>
#include <sys/wait.h>
#include <thread>

std::string nowText() {
  std::time_t now = std::time(nullptr);
  std::tm local{};
  localtime_r(&now, &local);
  char buffer[16];
  std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &local);
  return buffer;
}

void setResult(SharedState &sharedState, const std::string &label, bool passed,
               bool isDeploy,
               std::filesystem::file_time_type deployTime =
                   std::filesystem::file_time_type{}) {
  std::lock_guard<std::mutex> lock(sharedState.mutex);
  sharedState.result.resultPassed = passed;
  if (isDeploy && passed) {
    sharedState.lastDeploy = deployTime;
  }
  sharedState.result.resultLabel = label;
  sharedState.result.resultTime = nowText();
}

void addLine(SharedState &sharedState, const std::string &line) {
  std::lock_guard<std::mutex> lock(sharedState.mutex);
  sharedState.lines.push_back(line);
}

bool runNormalAction(SharedState &sharedState, const std::string &command) {
  FILE *pipe = popen((command + " 2>&1").c_str(), "r");
  if (pipe == nullptr) {
    addLine(sharedState, "ERROR: pipe is nullptr. command did not start");
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

  return exitcode == 0;
}

bool runInteractiveAction(const std::string &command) {
  def_prog_mode();
  endwin();
  int status = std::system(command.c_str());
  reset_prog_mode();
  clearok(curscr, TRUE);
  int exitcode = -1;
  if (WIFEXITED(status)) {
    exitcode = WEXITSTATUS(status);
  }

  return exitcode == 0;
}

void stopCommand(SharedState &sharedState) {
  if (!sharedState.busy) {
    addLine(sharedState, "nothing to stop");
    return;
  }
  int result = std::system("pkill -f \"[m]premote\" > /dev/null 2>&1");
  if (result == 0) {
    addLine(sharedState, "stopped");
  } else {
    addLine(sharedState, "nothing stopped, no running mpremote found");
  }
}

void selectAnAction(SharedState &sharedState,
                    const std::vector<Action> &actions, int selAction,
                    std::thread &worker) {
  std::string command = actions.at(selAction).command;
  std::string label = actions.at(selAction).label;
  ActionKind actionKind = actions.at(selAction).actionKind;

  if (actionKind == ActionKind::Stop) {
    stopCommand(sharedState);
    return;
  } else if (sharedState.busy) {
    addLine(sharedState, "busy, wait for the current command");
    return;
  } else if (actionKind == ActionKind::Interactive) {
    addLine(sharedState, "repl opened, exit with ctrl-x");
    bool resultPassed = runInteractiveAction(command);
    setResult(sharedState, label, resultPassed, false);
    return;
  }

  addLine(sharedState, command);
  if (worker.joinable()) {
    worker.join();
  }

  std::filesystem::file_time_type deployTime =
      std::filesystem::file_time_type::clock::now();
  sharedState.busy = true;

  worker =
      std::thread([&sharedState, command, label, deployTime, actionKind]() {
        bool resultPassed = runNormalAction(sharedState, command);
        setResult(sharedState, label, resultPassed,
                  actionKind == ActionKind::Deploy, deployTime);
        sharedState.busy = false;
      });
}
