#include "AppState.hpp"
#include "Colors.hpp"
#include "Help.hpp"
#include <functional>
#include <ncurses.h>
#include <random>
#include <string>
#include <vector>

std::vector<Sparkle> getSparkles(const std::string &path) {
  std::vector<Sparkle> sparkles;
  int max = 100;
  std::mt19937 rng(std::hash<std::string>{}(path));
  std::uniform_real_distribution<float> pos(0.0f, 1.0f);
  std::uniform_int_distribution<int> phase(0, 99);
  for (int i = 0; i < max; i++) {
    float x = pos(rng);
    float y = pos(rng);
    int p = phase(rng);
    sparkles.push_back({x, y, p});
  }
  return sparkles;
}

std::vector<std::string> getPhases() { return {"·", "˚", "⋆", "✦", "⋆", "˚"}; }

std::string getSparkleString(int phase, int step) {
  std::vector<std::string> phases = getPhases();
  return phases.at((step + phase) % phases.size());
}

int getStep(std::chrono::steady_clock::time_point startTime) {
  const int stepThreshold = 400;
  auto elapsed = std::chrono::steady_clock::now() - startTime;
  long long ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
  return intpls(ms / stepThreshold);
}

int getMaxSparkles(WINDOW *w, std::size_t size) {
  int density = 40; // tiles per sparkle
  return std::min(((getmaxx(w) * getmaxy(w)) / density), intpls(size));
}

int getRow(float y, WINDOW *w) { return (1 + y * (getmaxy(w) - 2)); }

int getCol(float x, WINDOW *w) {
  return (2 + x * (std::max(0, getmaxx(w) - 4)));
}

ColorPairs getSparkleColor(const Status &status,
                           const SharedState &sharedState) {
  if (sharedState.busy == true) {
    return ColorPairs::PAIR_WARN;
  } else if (status.deviceFound) {
    return ColorPairs::PAIR_GOOD;
  }
  return ColorPairs::PAIR_ERROR;
}
