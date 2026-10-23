#pragma once

namespace game {

inline int normalizedZoneSize(int size) { return size > 0 ? size : 1; }

/** Tile coordinate for index i in a zone centered on `center`. */
inline int zoneTileCoord(int center, int size, int index) {
  const auto zoneSize = normalizedZoneSize(size);
  return center + index - zoneSize / 2;
}

inline bool tileInCenteredZone(
    int tileX, int tileY, int centerX, int centerY, int zoneW, int zoneH) {
  zoneW = normalizedZoneSize(zoneW);
  zoneH = normalizedZoneSize(zoneH);
  const auto minX = centerX - zoneW / 2;
  const auto minY = centerY - zoneH / 2;
  return tileX >= minX && tileX < minX + zoneW && tileY >= minY && tileY < minY + zoneH;
}

} // namespace game
