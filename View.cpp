#include "View.hpp"
#include "AppState.hpp"
#include "Colors.hpp"
#include "Header.hpp"
#include "Sparkler.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <ncurses.h>
#include <string>
#include <vector>

int getInnerWidth(WINDOW *w) { return std::max(0, getmaxx(w) - 4); }

void drawLine(WINDOW *w, int row, int col, const ColoredLine &coloredLine,
              int maxl = -1) {
  wattron(w, COLOR_PAIR(coloredLine.color));
  mvwprintw(w, row, col, "%.*s", maxl, coloredLine.label.c_str());
  wattroff(w, COLOR_PAIR(coloredLine.color));
}

void drawStatusLines(WINDOW *w, const std::vector<ColoredLine> &coloredLines) {
  int start = 2, free = 0;
  int width = getInnerWidth(w);
  int right = start + width;
  for (const ColoredLine &coloredLine : coloredLines) {
    free += coloredLine.label.size();
  }
  free = width - free;
  int gaps = std::max(1, static_cast<int>(coloredLines.size() - 1));
  int gap = std::max(1, free / gaps);
  int col = start;
  int xtracol = free < gaps ? 0 : free % gaps;
  for (const ColoredLine &coloredLine : coloredLines) {
    if (right - col <= 0)
      break;
    drawLine(w, 1, col, coloredLine, right - col);
    col += coloredLine.label.size() + gap;
    if (xtracol > 0) {
      col++;
      xtracol--;
    }
  }
}

ColoredLine getPathLine(const Status &status) {
  return {status.path, ColorPairs::PAIR_ALT};
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
  int maxl = getInnerWidth(w);
  for (int i = start; i < lines; i++) {
    mvwprintw(w, i - start + 1, 2, "%.*s", maxl, logLines.at(i).c_str());
  }
}

void drawStatus(WINDOW *w, const Status &status,
                const SharedState &sharedState) {
  std::vector<ColoredLine> statuses = {
      getResultLine(sharedState), getBusyLine(sharedState),
      getDeviceLine(status), getFilesLine(status)};
  drawStatusLines(w, statuses);
}

void drawHeader(WINDOW *w, const Status &status, const SharedState &sharedState,
                const std::vector<Sparkle> &sparkles,
                std::chrono::steady_clock::time_point startTime) {
  int inw = getInnerWidth(w), maxSparkles = getMaxSparkles(w, sparkles.size());
  int iconStart = 4, margin = inw / 10, right = 2 + inw;

  ColorPairs col = getSparkleColor(status, sharedState);

  for (int i = 0; i < maxSparkles; i++) {
    const Sparkle &s = sparkles.at(i);
    ColoredLine l = {getSparkleString(s.phase, getStep(startTime)), col};
    drawLine(w, getRow(s.y, w), getCol(s.x, w), l);
  }

  std::vector<ColoredLine> icon = getIcon();
  if (iconStart + getIconWidth() < right) {
    for (size_t i = 0; i < icon.size(); i++) {
      drawLine(w, static_cast<int>(i) + 2, iconStart, icon.at(i));
    }
  }
  ColoredLine label = {"pico dashboard", PAIR_NONE}, path = getPathLine(status);
  int sCol = getIconWidth() + iconStart + margin,
      cols = sCol + static_cast<int>(path.label.size(), label.label.size());
  if (cols < right) {
    drawLine(w, 3, sCol, label, inw);
    drawLine(w, 4, sCol, path, inw);
  }
}
