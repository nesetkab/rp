#include "AppState.hpp"
#include <functional>
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
