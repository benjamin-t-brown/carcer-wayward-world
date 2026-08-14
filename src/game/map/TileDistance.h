#pragma once

namespace game {

/** Chebyshev (king-move) distance between two tile coordinates. */
int chebyshevDistance(int x0, int y0, int x1, int y1);

/** True when the tiles are Chebyshev distance 1 (including diagonals). */
bool isChebyshevAdjacent(int x0, int y0, int x1, int y1);

} // namespace game
