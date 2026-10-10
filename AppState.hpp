#pragma once
#include <string>

struct Action {
  std::string label;
  char key;
  std::string command;
};

struct Status {
  std::string status;
};
