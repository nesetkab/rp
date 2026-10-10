#pragma once
#include <mutex>
#include <string>
#include <vector>

struct Action {
  std::string label;
  char key;
  std::string command;
};

struct Status {
  std::string status;
};

struct SharedLog {
  std::vector<std::string> lines;
  std::mutex mutex;
};
