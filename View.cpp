#include "View.hpp"

auto lastCheck = std::chrono::steady_clock::now();

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

ColoredLine drawPath(const Status &status) { return {status.path, PAIR_NONE}; }

ColoredLine drawResult(const SharedState &sharedState) {
  if (sharedState.result.resultLabel.empty()) {
    return {"no command yet", PAIR_NONE};
  }
  std::string label = sharedState.result.resultLabel +
                      (sharedState.result.resultPassed ? " PASS " : " FAIL ") +
                      sharedState.result.resultTime;
  ColorPairs color = sharedState.result.resultPassed ? PAIR_GOOD : PAIR_ERROR;
  return {label, color};
}

ColoredLine drawBusy(const SharedState &sharedState) {
  std::string label = sharedState.busy ? "busy" : "free";
  ColorPairs color = sharedState.busy ? PAIR_WARN : PAIR_GOOD;
  return {label, color};
}

ColoredLine drawDevice(const Status &status) {
  std::string label = status.deviceFound ? "found" : "not found";
  ColorPairs color = status.deviceFound ? PAIR_GOOD : PAIR_ERROR;
  return {label, color};
}

ColoredLine drawFiles(const Status &status) {
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

void drawStatus(WINDOW *w, const Status &status,
                const SharedState &sharedState) {
  std::vector<ColoredLine> statuses = {
      drawPath(status), drawResult(sharedState), drawBusy(sharedState),
      drawDevice(status), drawFiles(status)};
  for (size_t i = 0; i < statuses.size(); i++) {
    drawColoredLine(w, static_cast<int>(i) + 1, statuses.at(i));
  }
}
