#pragma once
#include "AppState.hpp"
#include <string>
#include <thread>
#include <vector>

void runAnAction(std::vector<std::string> &log, const std::string &command);
void selectAnAction(SharedState &sharedState,
                    const std::vector<Action> &actions, int selAction,
                    std::thread &worker);
