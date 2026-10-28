#include "Header.hpp"
#include "AppState.hpp"
#include "Colors.hpp"
#include <string>
#include <vector>

std::vector<ColoredLine> getTitle() {
  return {{"██▄████ ██▄████▄ ", PAIR_ICON1},
          {"██▀     ██▀   ▀██", PAIR_ICON2},
          {"██      ██▄   ▄██", PAIR_ICON3},
          {"██      ██▀████▀ ", PAIR_ICON4},
          {"        ██       ", PAIR_ICON5}};
}
