#include "Terminal.hpp"
#include "Colors.hpp"
#include <clocale>
#include <ncurses.h>

Terminal::Terminal() {
  setlocale(LC_ALL, "");
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
    init_pair(PAIR_ICON1, 212, -1);
    init_pair(PAIR_ICON2, 177, -1);
    init_pair(PAIR_ICON3, 141, -1);
    init_pair(PAIR_ICON4, 111, -1);
    init_pair(PAIR_ICON5, 117, -1);
    init_pair(PAIR_ALT, 244, -1);
  }
}

Terminal::~Terminal() {
  curs_set(1);
  endwin();
}
