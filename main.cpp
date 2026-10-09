#include <ctime>
#include <string>
#include <ncurses.h>
#include <array>
#include <vector>

struct Action {
  std::string label;
  char key;
  std::string command;
};

struct Status {
  std::string status;
};

void drawActions(WINDOW* w, const std::vector<Action>& actions, int sel) {
    int maxy = getmaxy(w);
    for (size_t i = 0; i < actions.size(); i++) {
      int in = static_cast<int>(i);
      if (in >= maxy - 2) break;
      if (in == sel) wattron(w, A_REVERSE);
      mvwprintw(w, in+1, 2, "[%c] %s", actions.at(i).key, actions.at(i).label.c_str());
      wattroff(w, A_REVERSE);
    }
}

void drawLogs(WINDOW* w, const std::vector<std::string>& logLines) {
  int visible = getmaxy(w)-2;
  int lines = static_cast<int>(logLines.size());
  int start = std::max(0, lines-visible);
  for (int i = start; i < lines; i++) {
    mvwprintw(w, i-start+1, 2, "%s", logLines.at(i).c_str());
  }
}

void drawStatus(WINDOW* w, const Status& status) {
  mvwprintw(w, 1, 2, "%s", status.status.c_str());
}

int main() {
  std::vector<Action> actions = {{"run", 'r', "make run"}, {"deploy", 'd', "make deploy"}, {"repl", 'p', "make repl"}, {"ls", 'l', "make ls"}, {"reset", 'b', "make reset"}};
  int selAction = 0;
  std::vector<std::string> logLines;
  Status status = {"online"};
  
  initscr();
  cbreak();
  noecho();
  curs_set(0);

  int rows = 0;
  int cols = 0;
  getmaxyx(stdscr, rows, cols);
 
  int thirds = rows/3;
  WINDOW* top = newwin(thirds, cols, 0, 0);
  WINDOW* mid = newwin(thirds, cols, thirds, 0);
  WINDOW* bot = newwin(rows - 2 * thirds, cols, 2 * thirds, 0);
  std::array<WINDOW*, 3> windows = {top, mid, bot};
  

  keypad(top, TRUE);
  bool dash = true;
  char buffer[16];
  int lastkey = ERR;
  wtimeout(top, 100);
  while (dash) {
    for ( WINDOW* window : windows ) {
      werase(window);
      box(window, 0, 0);
    }

    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local);

    drawActions(top, actions, selAction); 
    drawLogs(mid, logLines);
    drawStatus(bot, status);

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
      case KEY_UP:
        if (selAction > 0) { 
          selAction--;
        }
        break;
      case KEY_DOWN:
        if (selAction+1 < static_cast<int>(actions.size())) {
          selAction++;
        }
        break;
      case '\n':
      case '\r':
      case KEY_ENTER:
      case ' ':
        logLines.push_back("Selected: " + actions.at(selAction).command);
        break;
      default:
        for (size_t i = 0; i < actions.size(); i++) {
          if (actions.at(i).key == key) {
            selAction = static_cast<int>(i);
            logLines.push_back("Selected: " + actions.at(selAction).command);
          }
        }
        lastkey = key;
        break;
    }
  }
  for ( WINDOW* w : windows ) {
    delwin(w);
  }
  endwin();
}
