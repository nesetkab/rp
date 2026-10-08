#include <ctime>
#include <string>
#include <ncurses.h>
#include <array>

int main() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);

  int rows = 0;
  int cols = 0;
  getmaxyx(stdscr, rows, cols);
 
  WINDOW* top = newwin(rows/3, cols, 0, 0);
  WINDOW* mid = newwin(rows/3, cols, rows/3 + 1, 0);
  WINDOW* bot = newwin(rows/3, cols, 2 * rows/3 + 1, 0);
  std::array<WINDOW*, 3> windows = {top, mid, bot};
  box(top, 0, 0);
  for ( WINDOW* window : windows ) {
    werase(window);
    box(window, 0, 0);
  }

  keypad(top, TRUE);
  bool dash = true;
  char buffer[16];
  int lastkey = ERR;
  wtimeout(top, 100);
  while (dash) {

    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
    mvwprintw(top, 1, 2, "%s", buffer);
    if (lastkey != ERR) {
      mvwprintw(top, 2, 2, "%s", keyname(lastkey));
    }

    for ( WINDOW* w : windows ) {
      wnoutrefresh(w);
    }
    doupdate();

    int key = wgetch(top);
    switch (key) {
      case ERR:
        break;
      case 'q':
        dash = false;
        break;
      default:
        lastkey = key;
        break;
    }
  }
  for ( WINDOW* w : windows ) {
    delwin(w);
  }
  endwin();
}
