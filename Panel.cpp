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

    int y = 0, x = 0;
    getmaxyx(win, y, x);
    mvwprintw(win, 0, 0, "╭");
    mvwprintw(win, y, 0, "╰");
    mvwprintw(win, 0, x, "╮");
    mvwprintw(win, y, x, "╯");
  }
}

WINDOW *Panel::get() const { return win; }

void Panel::resize(const Rect &r) {
  wresize(win, r.height, r.width);
  mvwin(win, r.row, r.col);
}
