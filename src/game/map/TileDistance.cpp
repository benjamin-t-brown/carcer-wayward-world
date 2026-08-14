#include "game/map/TileDistance.h"

namespace game {

int chebyshevDistance(int x0, int y0, int x1, int y1) {
  const auto dx = x0 < x1 ? x1 - x0 : x0 - x1;
  const auto dy = y0 < y1 ? y1 - y0 : y0 - y1;
  return dx > dy ? dx : dy;
}

bool isChebyshevAdjacent(int x0, int y0, int x1, int y1) {
  return chebyshevDistance(x0, y0, x1, y1) == 1;
}

} // namespace game
