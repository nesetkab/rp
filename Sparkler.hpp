#include "AppState.hpp"
#include <ncurses.h>
#include <string>
#include <vector>

std::vector<Sparkle> getSparkles(const std::string &path);
std::string getSparkleString(int phase, int step);
int getStep(std::chrono::steady_clock::time_point startTime);
int getMaxSparkles(WINDOW *w, std::size_t size);
int getRow(float y, WINDOW *w);
int getCol(float x, WINDOW *w);
