#include "Header.hpp"
#include "AppState.hpp"
#include "Colors.hpp"
#include <string>
#include <vector>

std::vector<ColoredLine> getIcon() {
  return {{"██▄████ ██▄████▄ ", PAIR_ICON1},
          {"██▀     ██▀   ▀██", PAIR_ICON2},
          {"██      ██▄   ▄██", PAIR_ICON3},
          {"██      ██▀████▀ ", PAIR_ICON4},
          {"        ██       ", PAIR_ICON5}};
}

int getIconWidth() {
  // one of the block characters is 3 bytes
  // so .size() returns something larger than the actual length
  // this is a workaround c:
  std::string line = getIcon().at(0).label;
  int length = 0;
  for (unsigned char c : line) {
    if ((c & 0xC0) != 0x80) {
      length++;
    }
  }
  return length;
}
