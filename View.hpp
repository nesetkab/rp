#pragma once
#include "AppState.hpp"
#include <ncurses.h>
#include <string>
#include <vector>

ColoredLine getPathLine(const Status &status);
void drawActions(WINDOW *w, const std::vector<Action> &actions, int sel);
void drawLogs(WINDOW *w, const std::vector<std::string> &logLines);
void drawStatus(WINDOW *w, const Status &status,
                const SharedState &sharedState);
void drawHeader(WINDOW *w, const Status &status, const SharedState &sharedState,
                const std::vector<Sparkle> &sparkles,
                std::chrono::steady_clock::time_point startTime);
