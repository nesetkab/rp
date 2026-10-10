#include "View.hpp"
#include "AppState.hpp"
#include "Colors.hpp"
#include <ncurses.h>
#include <string>
#include <vector>

void drawActions(WINDOW *w, const std::vector<Action> &actions, int sel) {
  int maxy = getmaxy(w);
  for (size_t i = 0; i < actions.size(); i++) {
    int in = static_cast<int>(i);
    if (in >= maxy - 2)
      break;
    if (in == sel)
      wattron(w, A_REVERSE | A_BOLD);
    mvwprintw(w, in + 1, 2, "[%c] %s", actions.at(i).key,
              actions.at(i).label.c_str());
    wattroff(w, A_REVERSE | A_BOLD);
  }
}

void drawLogs(WINDOW *w, const std::vector<std::string> &logLines) {
  int visible = getmaxy(w) - 2;
  int lines = static_cast<int>(logLines.size());
  int start = std::max(0, lines - visible);
  int maxl = std::max(0, getmaxx(w) - 3);
  for (int i = start; i < lines; i++) {
    mvwprintw(w, i - start + 1, 2, "%.*s", maxl, logLines.at(i).c_str());
  }
}

void drawStatus(WINDOW *w, const Status &status) {
  wattron(w, COLOR_PAIR(PAIR_GOOD));
  mvwprintw(w, 1, 2, "%s", status.status.c_str());
  wattroff(w, COLOR_PAIR(PAIR_GOOD));
}
