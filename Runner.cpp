#include "Runner.hpp"

void runAnAction(std::vector<std::string> &log, const std::string &command) {
  FILE *pipe = popen((command + " 2>&1").c_str(), "r");
  if (pipe == nullptr) {
    log.push_back("err, pipe is nullptr. command did not start");
    return;
  }

  char buffer[512];
  while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    std::string line = buffer;
    if (!line.empty() && line.back() == '\n') {
      line.pop_back();
    }
    log.push_back(line);
  }

  int status = pclose(pipe);
  int exitcode = -1;
  if (WIFEXITED(status)) {
    exitcode = WEXITSTATUS(status);
  }
  log.push_back("finished! exit code " + std::to_string(exitcode));
}

void selectAnAction(std::vector<std::string> &log,
                    const std::vector<Action> &actions, int selAction) {
  log.push_back("Selected: " + actions.at(selAction).command);
  runAnAction(log, actions.at(selAction).command);
}
