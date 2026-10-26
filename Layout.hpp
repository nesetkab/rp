#pragma once

#include "AppState.hpp"
#include <vector>
struct Rect {
  int height = 0;
  int width = 0;
  int row = 0;
  int col = 0;
};

struct Layout {
  Rect header;
  Rect actions;
  Rect log;
  Rect status;
  bool showHeader = false;
};

Layout getLayout(int rows, int cols, const std::vector<Action> &action);
