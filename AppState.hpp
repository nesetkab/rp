#pragma once
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

enum class FilesState { Unknown, Deployed, Changed };

struct Action {
  std::string label;
  char key;
  std::string command;
};

struct Status {
  std::string path;
  bool deviceFound = false;
  FilesState filesFound = FilesState::Unknown;
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
};
