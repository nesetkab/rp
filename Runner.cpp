#include "Runner.hpp"

void selectAnAction(std::vector<std::string> &logLines,
                    const std::vector<Action> &actions, int selAction) {
  logLines.push_back("Selected: " + actions.at(selAction).command);
}
