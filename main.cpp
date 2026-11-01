#include "AppState.hpp"
#include "Config.hpp"
#include "Demo.hpp"
#include "Help.hpp"
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
#include <memory>
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
                    std::chrono::steady_clock::time_point startTime,
                    bool showHeader) {
  checkStatus(status, sharedState, config);
  drawActions(windows.at(1)->get(), config.actions, selAction);
  {
    std::lock_guard<std::mutex> lock(sharedState.mutex);
    drawLogs(windows.at(2)->get(), sharedState.lines);
    drawStatus(windows.at(3)->get(), status, sharedState);
    drawHeader(windows.at(0)->get(), status, sharedState, sparkles, startTime,
               showHeader);
  }
}

Layout resize(const Config &config, const std::array<Panel *, 4> w) {
  int rows = 0, cols = 0;
  getmaxyx(stdscr, rows, cols);
  Layout layout = getLayout(rows, cols, config.actions);
  w.at(0)->resize(layout.header);
  w.at(1)->resize(layout.actions);
  w.at(2)->resize(layout.log);
  w.at(3)->resize(layout.status);
  clearok(curscr, TRUE);
  return layout;
}

#ifndef RP_VERSION
#define RP_VERSION "dev"
#endif

void printUsage() {
  std::cout << "rp " RP_VERSION " - a terminal dashboard for MicroPython on the "
               "Raspberry Pi Pico\n\n"
               "usage:\n"
               "  rp                 use ./config.json\n"
               "  rp <config.json>   use another config file\n"
               "  rp --demo          try rp with a pretend pico (no hardware)\n"
               "  rp --version       print the version\n"
               "  rp --help          print this help\n";
}

int main(int argc, char *argv[]) {
  std::string arg = argc > 1 ? argv[1] : "";
  if (arg == "--help" || arg == "-h") {
    printUsage();
    return 0;
  }
  if (arg == "--version" || arg == "-v") {
    std::cout << "rp " RP_VERSION "\n";
    return 0;
  }
  try {
    std::unique_ptr<DemoProject> demo;
    std::string configPath = "config.json";
    if (arg == "--demo") {
      demo = std::make_unique<DemoProject>();
      configPath = demo->configPath();
    } else if (!arg.empty()) {
      configPath = arg;
    }
    Config config = loadConfig(configPath);
    int selAction = 0;
    Status status = {std::filesystem::current_path(), false,
                     FilesState::Unknown};
    SharedState sharedState;
    std::thread worker;
    Terminal terminal;
    if (demo) {
      for (const std::string &line : demo->welcomeLines()) {
        addLine(sharedState, line);
      }
    }

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
      for (int i = 0; i < intpls(windows.size()); i++) {
        bool showBox = true;
        if (i == 0) {
          showBox = layout.showHeader;
        }
        windows.at(i)->drawFrame(showBox);
      }

      drawEverything(windows, status, sharedState, config, selAction, sparkles,
                     startTime, layout.showHeader);

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
            if (selAction >= intpls(config.actions.size())) {
              selAction = 0;
            }
            layout = resize(config, windows);
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
          if (selAction + 1 < intpls(config.actions.size())) {
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
          layout = resize(config, windows);
          break;

        default:
          for (size_t i = 0; i < config.actions.size(); i++) {
            if (config.actions.at(i).key == key) {
              selAction = intpls(i);
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
