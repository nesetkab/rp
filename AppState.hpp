#pragma once
#include "Colors.hpp"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

enum class FilesState { Unknown, Deployed, Changed };

enum class ActionKind { Normal, Interactive, Deploy, Stop };

struct ColoredLine {
  std::string label;
  ColorPairs color;
};

struct Action {
  std::string label;
  char key;
  std::string command;
  ActionKind actionKind = ActionKind::Normal;
};

struct Config {
  std::vector<Action> actions;
  std::string devicePrefix;
};

struct Status {
  std::string path;
  bool deviceFound = false;
  FilesState filesFound = FilesState::Unknown;
  std::chrono::steady_clock::time_point lastCheck{};
};

struct Result {
  std::string resultLabel;
  bool resultPassed = false;
  std::string resultTime;
};

struct SharedState {
  std::vector<std::string> lines;
  std::mutex mutex;
  std::atomic<bool> busy{false};
  Result result;
  std::filesystem::file_time_type lastDeploy{};
};
