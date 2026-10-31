#include "Layout.hpp"
#include "AppState.hpp"
#include "Help.hpp"
#include <algorithm>

Layout getLayout(int rows, int cols, const std::vector<Action> &action) {
  int maxAcL = 0;
  if (!action.empty()) {
    for (const Action &a : action) {
      if (intpls(a.label.length()) > maxAcL)
        maxAcL = intpls(a.label.length());
    }
    maxAcL += 8; // margin for border n shiiii
  }
  maxAcL = std::max(12, maxAcL);

  const int headerToggleRows = 19, headerH = 8, statusH = 3;

  Rect header, actions, log, status;
  bool showHeader = false;

  header.width = cols;
  if (rows > headerToggleRows) {
    header.height = headerH;
    showHeader = true;
  } else {
    header.height = 1;
  }

  actions.height =
      std::max(1, rows - header.height - statusH); // total - header - status
  actions.width = maxAcL;
  actions.row = header.height;

  log.height = actions.height;
  log.width = cols - maxAcL;
  log.col = actions.width;
  log.row = header.height;

  status.height = statusH;
  status.width = cols;
  status.row = header.height + actions.height;
  return {header, actions, log, status, showHeader};
}
