#include "AppState.hpp"
#include "Panel.hpp"
#include "Runner.hpp"
#include "Terminal.hpp"
#include "View.hpp"
#include <array>
#include <ncurses.h>
#include <string>
#include <vector>

int main() {
  std::vector<Action> actions = {{"run", 'r', "make run"},
                                 {"deploy", 'd', "make deploy"},
                                 {"repl", 'p', "make repl"},
                                 {"ls", 'l', "make ls"},
                                 {"reset", 'b', "make reset"}};
  int selAction = 0;
  std::vector<std::string> logLines;
  Status status = {"online"};

  Terminal terminal;

  int rows = 0;
  int cols = 0;
  getmaxyx(stdscr, rows, cols);

  int thirds = rows / 3;
  Panel winA(thirds, cols, 0, 0, "actions");
  Panel winL(thirds, cols, thirds, 0, "logs");
  Panel winS(rows - 2 * thirds, cols, 2 * thirds, 0, "status");
  std::array<Panel *, 3> windows = {&winA, &winL, &winS};

  keypad(winA.get(), TRUE);
  bool dash = true;
  wtimeout(winA.get(), 100);
  while (dash) {
    for (Panel *p : windows) {
      p->drawFrame();
    }

    drawActions(winA.get(), actions, selAction);
    drawLogs(winL.get(), logLines);
    drawStatus(winS.get(), status);

    for (Panel *p : windows) {
      wnoutrefresh(p->get());
    }
    doupdate();

    int key = wgetch(winA.get());
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
      if (selAction + 1 < static_cast<int>(actions.size())) {
        selAction++;
      }
      break;
    case '\n':
    case '\r':
    case KEY_ENTER:
    case ' ':
      if (!actions.empty()) {
        selectAnAction(logLines, actions, selAction);
      }
      break;
    default:
      for (size_t i = 0; i < actions.size(); i++) {
        if (actions.at(i).key == key) {
          selAction = static_cast<int>(i);
          selectAnAction(logLines, actions, selAction);
          break;
        }
      }
      break;
    }
  }
}
