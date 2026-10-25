#include "View.hpp"
#include "Colors.hpp"
#include <algorithm>
#include <ncurses.h>
#include <string>
#include <vector>

struct ColoredLine {
  std::string label;
  ColorPairs color;
};

void drawColoredLine(WINDOW *w, int row, const ColoredLine &coloredLine) {
  int maxl = std::max(0, getmaxx(w) - 3);
  wattron(w, COLOR_PAIR(coloredLine.color));
  mvwprintw(w, row, 2, "%.*s", maxl, coloredLine.label.c_str());
  wattroff(w, COLOR_PAIR(coloredLine.color));
}

void drawColoredLines(WINDOW *w, const std::vector<ColoredLine> &coloredLines) {
  int maxl = std::max(0, getmaxx(w) - 3);
  int row = 1;
  for (const ColoredLine coloredLine : coloredLines) {
    wattron(w, COLOR_PAIR(coloredLine.color));
    mvwprintw(w, 1, row, "%.*s", maxl, coloredLine.label.c_str());
    wattroff(w, COLOR_PAIR(coloredLine.color));
    row += coloredLine.label.size() + 1;
  }
}

ColoredLine getPathLine(const Status &status) {
  return {status.path, PAIR_NONE};
}

ColoredLine getResultLine(const SharedState &sharedState) {
  if (sharedState.result.resultLabel.empty()) {
    return {"no command yet", PAIR_NONE};
  }
  std::string label = sharedState.result.resultLabel +
                      (sharedState.result.resultPassed ? " PASS " : " FAIL ") +
                      sharedState.result.resultTime;
  ColorPairs color = sharedState.result.resultPassed ? PAIR_GOOD : PAIR_ERROR;
  return {label, color};
}

ColoredLine getBusyLine(const SharedState &sharedState) {
  std::string label = sharedState.busy ? "busy" : "free";
  ColorPairs color = sharedState.busy ? PAIR_WARN : PAIR_GOOD;
  return {label, color};
}

ColoredLine getDeviceLine(const Status &status) {
  std::string label = status.deviceFound ? "found" : "not found";
  ColorPairs color = status.deviceFound ? PAIR_GOOD : PAIR_ERROR;
  return {label, color};
}

ColoredLine getFilesLine(const Status &status) {
  std::string label = "unknown";
  ColorPairs color = PAIR_NONE;
  switch (status.filesFound) {
    case FilesState::Deployed:
      color = PAIR_GOOD;
      label = "deployed";
      break;
    case FilesState::Changed:
      color = PAIR_WARN;
      label = "changed, not deployed";
      break;
    default:
      break;
  }
  return {label, color};
}

void drawActions(WINDOW *w, const std::vector<Action> &actions, int sel) {
  int maxy = getmaxy(w);
  for (size_t i = 0; i < actions.size(); i++) {
    int in = static_cast<int>(i);
    if (in >= maxy - 2)
      break;
    if (in == sel)
      wattron(w, A_REVERSE | A_ITALIC);
    mvwprintw(w, in + 1, 2, "[%c] %s", actions.at(i).key,
              actions.at(i).label.c_str());
    wattroff(w, A_REVERSE | A_ITALIC);
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

void drawStatus(WINDOW *w, const Status &status,
                const SharedState &sharedState) {
  std::vector<ColoredLine> statuses = {
      getPathLine(status), getResultLine(sharedState), getBusyLine(sharedState),
      getDeviceLine(status), getFilesLine(status)};
  drawColoredLines(w, statuses);
}
