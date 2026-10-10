#include "Terminal.hpp"
#include "Colors.hpp"
#include <ncurses.h>

Terminal::Terminal() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(PAIR_GOOD, COLOR_GREEN, -1);
    init_pair(PAIR_WARN, COLOR_YELLOW, -1);
    init_pair(PAIR_ERROR, COLOR_RED, -1);
  }
}

Terminal::~Terminal() {
  endwin();
  curs_set(1);
}
