#include "AppState.hpp"
#include "Config.hpp"
#include "Layout.hpp"
#include "Panel.hpp"
#include "Runner.hpp"
#include "Sparkler.hpp"
#include "StatusCheck.hpp"
#include "Terminal.hpp"
#include "View.hpp"
#include <array>
#include <exception>
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

void drawEverything(const std::array<Panel *, 4> &windows, Status &status,
                    SharedState &sharedState, const Config &config,
                    const int &selAction, const std::vector<Sparkle> &sparkles,
                    std::chrono::steady_clock::time_point startTime) {
  checkStatus(status, sharedState, config.devicePrefix);
  drawActions(windows.at(1)->get(), config.actions, selAction);
  {
    std::lock_guard<std::mutex> lock(sharedState.mutex);
    drawLogs(windows.at(2)->get(), sharedState.lines);
    drawStatus(windows.at(3)->get(), status, sharedState);
    drawHeader(windows.at(0)->get(), status, sharedState, sparkles, startTime);
  }
}

void resize(const Config &config, const std::array<Panel *, 4> w) {
  int rows = 0, cols = 0;
  getmaxyx(stdscr, rows, cols);
  Layout layout = getLayout(rows, cols, config.actions);
  w.at(0)->resize(layout.header);
  w.at(1)->resize(layout.actions);
  w.at(2)->resize(layout.log);
  w.at(3)->resize(layout.status);
  clearok(curscr, TRUE);
}

int main(int argc, char *argv[]) {
  try {
    std::string configPath = argc > 1 ? argv[1] : "config.json";

    Config config = loadConfig(configPath);
    int selAction = 0;
    Status status = {std::filesystem::current_path(), false,
                     FilesState::Unknown};
    SharedState sharedState;
    std::thread worker;
    Terminal terminal;

    std::vector<Sparkle> sparkles = getSparkles(getPathLine(status).label);
    auto startTime = std::chrono::steady_clock::now();

    // make the three windows
    int rows = 0, cols = 0;
    getmaxyx(stdscr, rows, cols);
    Layout layout = getLayout(rows, cols, config.actions);
    Panel winH(layout.header.height, layout.header.width, layout.header.row,
               layout.header.col, "header");
    Panel winA(layout.actions.height, layout.actions.width, layout.actions.row,
               layout.actions.col, "actions");
    Panel winL(layout.log.height, layout.log.width, layout.log.row,
               layout.log.col, "logs");
    Panel winS(layout.status.height, layout.status.width, layout.status.row,
               layout.status.col, "status");
    std::array<Panel *, 4> windows = {&winH, &winA, &winL, &winS};

    setupInput(winA.get());
    bool dash = true;

    while (dash) {
      for (Panel *p : windows) {
        p->drawFrame();
      }

      drawEverything(windows, status, sharedState, config, selAction, sparkles,
                     startTime);

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

        case 'R':
          try {
            config = loadConfig(configPath);
            if (selAction >= static_cast<int>(config.actions.size())) {
              selAction = 0;
            }
            resize(config, windows);
            addLine(sharedState, "config reloaded");
          } catch (const std::exception &e) {
            addLine(sharedState,
                    "reload config error: " + std::string(e.what()));
          }
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

        case KEY_RESIZE:
          resize(config, windows);
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
