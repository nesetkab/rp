#pragma once

#include <ncurses.h>
#include <string>

class Panel {
public:
  Panel(int h, int w, int srow, int scol, const std::string &t);
  ~Panel();
  Panel(const Panel &) = delete;
  Panel &operator=(const Panel &) = delete;
  void drawFrame();
  WINDOW *get() const;

private:
  WINDOW *win;
  std::string title;
};
