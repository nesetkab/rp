#include "Layout.hpp"
#include <algorithm>

Layout getLayout(int rows, int cols, int actionsW) {
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
  actions.width = actionsW;
  actions.row = header.height;

  log.height = actions.height;
  log.width = cols - actionsW;
  log.col = actions.width;
  log.row = header.height;

  status.height = statusH;
  status.width = cols;
  status.row = header.height + actions.height;
  return {header, actions, log, status, showHeader};
}
