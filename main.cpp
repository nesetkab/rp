#include <ctime>
#include <string>
#include <ncurses.h>
#include <array>
#include <vector>
#include <algorithm>
#include <stdexcept>

enum ColorPairs {
  PAIR_GOOD = 1,
  PAIR_WARN,
  PAIR_ERROR,
};

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
      if (in == sel) wattron(w, A_REVERSE | A_BOLD);
      mvwprintw(w, in+1, 2, "[%c] %s", actions.at(i).key, actions.at(i).label.c_str());
      wattroff(w, A_REVERSE | A_BOLD);
    }
}

void drawLogs(WINDOW* w, const std::vector<std::string>& logLines) {
  int visible = getmaxy(w)-2;
  int lines = static_cast<int>(logLines.size());
  int start = std::max(0, lines-visible);
  int maxl = std::max(0, getmaxx(w)-3);
  for (int i = start; i < lines; i++) {
    mvwprintw(w, i-start+1, 2, "%.*s", maxl, logLines.at(i).c_str());
  }
}

void drawStatus(WINDOW* w, const Status& status) {
  wattron(w, COLOR_PAIR(PAIR_GOOD));
  mvwprintw(w, 1, 2, "%s", status.status.c_str());
  wattroff(w, COLOR_PAIR(PAIR_GOOD));
}


class Terminal {
  public:
    Terminal();
    ~Terminal();
    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;
};

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

class Panel {
  public:
    Panel(int h, int w, int srow, int scol, const std::string& t);
    ~Panel();
    Panel(const Panel&) = delete;
    Panel& operator=(const Panel&) = delete;
    void drawFrame();
    WINDOW* get() const;

  private:
    WINDOW* win;
    std::string title;
};

Panel::Panel(int h, int w, int srow, int scol, const std::string& t) : win(newwin(h, w, srow, scol)), title(t) {}

Panel::~Panel() {
  delwin(win);
}

void Panel::drawFrame() {
  werase(win);
  box(win, 0, 0);
  mvwprintw(win, 0, 2, " %s ", title.c_str());
}

WINDOW* Panel::get() const {
  return win;
}


int main() {
  std::vector<Action> actions = {{"run", 'r', "make run"}, {"deploy", 'd', "make deploy"}, {"repl", 'p', "make repl"}, {"ls", 'l', "make ls"}, {"reset", 'b', "make reset"}};
  int selAction = 0;
  std::vector<std::string> logLines;
  Status status = {"online"};
  
  Terminal terminal; 

  int rows = 0;
  int cols = 0;
  getmaxyx(stdscr, rows, cols);
 
  int thirds = rows/3;
  Panel winA(thirds, cols, 0, 0, "actions");
  Panel winL(thirds, cols, thirds, 0, "logs");
  Panel winS(rows - 2 * thirds, cols, 2 * thirds, 0, "status");
  std::array<Panel*, 3> windows = {&winA, &winL, &winS};

  keypad(winA.get(), TRUE);
  bool dash = true;
  wtimeout(winA.get(), 100);
  while (dash) { 
    for (Panel* p : windows) {
      p->drawFrame();
    }

    drawActions(winA.get(), actions, selAction); 
    drawLogs(winL.get(), logLines);
    drawStatus(winS.get(), status);

    for (Panel* p : windows) {
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
        if (selAction+1 < static_cast<int>(actions.size())) {
          selAction++;
        }
        break;
      case '\n':
      case '\r':
      case KEY_ENTER:
      case ' ':
        if (!actions.empty()) {
          logLines.push_back("Selected: " + actions.at(selAction).command);
        }
        break;
      default:
        for (size_t i = 0; i < actions.size(); i++) {
          if (actions.at(i).key == key) {
            selAction = static_cast<int>(i);
            logLines.push_back("Selected: " + actions.at(selAction).command);
            break;
          }
        }
        break;
    }
  }
}
