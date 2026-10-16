#include "AppState.hpp"
#include "Config.hpp"
#include "Panel.hpp"
#include "Runner.hpp"
#include "StatusCheck.hpp"
#include "Terminal.hpp"
#include "View.hpp"
#include <array>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

void setupInput(WINDOW *w) {
  keypad(w, TRUE);
  wtimeout(w, 100);
}

void drawEverything(const std::array<Panel *, 3> &windows, Status &status,
                    SharedState &sharedState, const Config &config,
                    int selAction) {
  checkStatus(status, sharedState, config.devicePrefix);
  drawActions(windows.at(0)->get(), config.actions, selAction);
  {
    std::lock_guard<std::mutex> lock(sharedState.mutex);
    drawLogs(windows.at(1)->get(), sharedState.lines);
    drawStatus(windows.at(2)->get(), status, sharedState);
  }
}

int main() {
  try {
    std::string configPath = "config.json";
    Config config = loadConfig(configPath);
    int selAction = 0;
    Status status = {std::filesystem::current_path(), false,
                     FilesState::Unknown};
    SharedState sharedState;
    std::thread worker;
    Terminal terminal;

    // make the three windows
    int rows, cols = 0;
    getmaxyx(stdscr, rows, cols);
    int thirds = rows / 3;
    Panel winA(thirds, cols, 0, 0, "actions");
    Panel winL(thirds, cols, thirds, 0, "logs");
    Panel winS(rows - 2 * thirds, cols, 2 * thirds, 0, "status");
    std::array<Panel *, 3> windows = {&winA, &winL, &winS};

    setupInput(winA.get());
    bool dash = true;

    while (dash) {
      for (Panel *p : windows) {
        p->drawFrame();
      }

      drawEverything(windows, status, sharedState, config, selAction);

      for (Panel *p : windows) {
        wnoutrefresh(p->get());
      }
      doupdate();

      int key = wgetch(winA.get());
      switch (key) {
        case ERR:
          break;

        case 'q':
          stopCommand(sharedState);
          dash = false;
          break;

        case KEY_UP:
          if (selAction > 0) {
            selAction--;
          }
          break;

        case KEY_DOWN:
          if (selAction + 1 < static_cast<int>(config.actions.size())) {
            selAction++;
          }
          break;

        case '\n':
        case '\r':
        case KEY_ENTER:
        case ' ':
          if (!config.actions.empty()) {
            selectAnAction(sharedState, config.actions, selAction, worker);
          }
          break;

        default:
          for (size_t i = 0; i < config.actions.size(); i++) {
            if (config.actions.at(i).key == key) {
              selAction = static_cast<int>(i);
              selectAnAction(sharedState, config.actions, selAction, worker);
              break;
            }
          }
          break;
      }
    }
    if (worker.joinable()) {
      worker.join();
    }
  } catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << '\n';
    return 1;
  }
}
