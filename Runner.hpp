#pragma once
#include "AppState.hpp"
#include <string>
#include <vector>

void selectAnAction(std::vector<std::string> &logLines,
                    const std::vector<Action> &actions, int selAction);
