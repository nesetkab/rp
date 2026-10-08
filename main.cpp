#include <ctime>
#include <string>
#include <ncurses.h>

int main() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  bool dash = true;
  char buffer[16];
  int lastkey = ERR;
  timeout(100);
  while (dash) {
    erase();

    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
    mvprintw(10, 10, "%s", buffer);
    if (lastkey != ERR) {
      mvprintw(0, 0, "keyyyy: %s", keyname(lastkey));
    }

    refresh();

    int key = getch();
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
  endwin();
}
