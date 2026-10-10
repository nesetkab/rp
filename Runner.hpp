#pragma once
#include "AppState.hpp"
#include <cstdio>
#include <string>
#include <sys/wait.h>
#include <vector>

void runAnAction(std::vector<std::string> &log, const std::string &command);
void selectAnAction(std::vector<std::string> &logLines,
                    const std::vector<Action> &actions, int selAction);
