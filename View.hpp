#pragma once
#include "AppState.hpp"
#include "Colors.hpp"
#include <algorithm>
#include <chrono>
#include <ncurses.h>
#include <string>
#include <vector>

void drawActions(WINDOW *w, const std::vector<Action> &actions, int sel);
void drawLogs(WINDOW *w, const std::vector<std::string> &logLines);
void drawStatus(WINDOW *w, const Status &status,
                const SharedState &sharedState);
