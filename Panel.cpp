#include "Panel.hpp"
#include "Layout.hpp"

Panel::Panel(int h, int w, int srow, int scol, const std::string &t)
    : win(newwin(h, w, srow, scol)), title(t) {}

Panel::~Panel() { delwin(win); }

void Panel::drawFrame(const bool &showBox) {
  werase(win);
  if (showBox) {
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " %s ", title.c_str());
  }
}

WINDOW *Panel::get() const { return win; }

void Panel::resize(const Rect &r) {
  wresize(win, r.height, r.width);
  mvwin(win, r.row, r.col);
}
