#pragma once
#include "AppState.hpp"
#include <ncurses.h>
#include <thread>
#include <vector>

void stopCommand(SharedState &sharedState);
void selectAnAction(SharedState &sharedState,
                    const std::vector<Action> &actions, int selAction,
                    std::thread &worker);
