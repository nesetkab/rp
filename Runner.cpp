#include "Runner.hpp"
#include "AppState.hpp"
#include <atomic>
#include <cstdio>
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
