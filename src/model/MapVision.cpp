#include "model/MapVision.h"
#include "model/MapWalkability.h"
#include <cstdint>
#include <cstdlib>

namespace model {
namespace {

int tileIndex(const MapInstance& map, int x, int y) {
  if (map.width <= 0) {
    return -1;
  }
  return y * map.width + x;
}

bool inBounds(const MapInstance& map, int x, int y) {
  return x >= 0 && y >= 0 && x < map.width && y < map.height;
}

void clearAllVisible(MapInstance& map) {
  for (size_t li = 0; li < map.tiles.size(); li++) {
    auto& layerTiles = map.tiles[li];
    for (size_t ti = 0; ti < layerTiles.size(); ti++) {
      layerTiles[ti].isVisible = false;
    }
  }
}

// Sync isVisible / isExplored across every layer at (x, y), including empty tiles.
void markCellVisibleAndExplored(MapInstance& map, int x, int y) {
  if (!inBounds(map, x, y)) {
    return;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return;
  }

  for (size_t li = 0; li < map.tiles.size(); li++) {
    auto& layerTiles = map.tiles[li];
    if (index >= static_cast<int>(layerTiles.size())) {
      continue;
    }
    auto& tile = layerTiles[static_cast<size_t>(index)];
    tile.isVisible = true;
    tile.isExplored = true;
  }
}

void castVisibilityRay(MapInstance& map,
                       int x1,
                       int y1,
                       int x2,
                       int y2,
                       const db::Database& database) {
  auto visibility = true;

  const auto dx = std::abs(x2 - x1);
  const auto sx = x1 < x2 ? 1 : -1;
  const auto dy = std::abs(y2 - y1);
  const auto sy = y1 < y2 ? 1 : -1;
  auto err = (dx > dy ? dx : -dy) / 2;

  while (true) {
    if (!inBounds(map, x1, y1)) {
      // Prefer clean exit over TS ctr>10 + continue-without-advance quirk.
      break;
    }

    if (visibility) {
      markCellVisibleAndExplored(map, x1, y1);
    }

    if (!isDestinationSeeThrough(map, x1, y1, database)) {
      visibility = false;
    }

    if (x1 == x2 && y1 == y2) {
      break;
    }

    const auto e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y1 += sy;
    }
  }
}

// Perimeter Bresenham leaves gaps along opaque wall faces (a ray that hits one wall
// tile stops, so neighbors on the same wall never get their own hit). Light any
// opaque cell in range that touches a visible see-through cell.
void lightOpaqueWallsBesideVisibleFloors(MapInstance& map,
                                         int playerX,
                                         int playerY,
                                         int boxSize,
                                         const db::Database& database) {
  for (auto y = playerY - boxSize; y <= playerY + boxSize; y++) {
    for (auto x = playerX - boxSize; x <= playerX + boxSize; x++) {
      if (!isInPlayerVisionRange(x - playerX, y - playerY, boxSize)) {
        continue;
      }
      if (!inBounds(map, x, y)) {
        continue;
      }
      const auto index = tileIndex(map, x, y);
      if (index < 0) {
        continue;
      }
      // Already visible from a ray — nothing to do.
      auto alreadyVisible = false;
      if (mapHasLayer(map.tiles, 0) &&
          index < static_cast<int>(map.tiles[0].size())) {
        alreadyVisible = map.tiles[0][static_cast<size_t>(index)].isVisible;
      }
      if (alreadyVisible) {
        continue;
      }
      if (isDestinationSeeThrough(map, x, y, database)) {
        continue;
      }

      auto besideVisibleFloor = false;
      for (auto dy = -1; dy <= 1 && !besideVisibleFloor; dy++) {
        for (auto dx = -1; dx <= 1; dx++) {
          if (dx == 0 && dy == 0) {
            continue;
          }
          const auto nx = x + dx;
          const auto ny = y + dy;
          if (!inBounds(map, nx, ny)) {
            continue;
          }
          const auto nIndex = tileIndex(map, nx, ny);
          if (nIndex < 0 || !mapHasLayer(map.tiles, 0) ||
              nIndex >= static_cast<int>(map.tiles[0].size())) {
            continue;
          }
          if (!map.tiles[0][static_cast<size_t>(nIndex)].isVisible) {
            continue;
          }
          if (!isDestinationSeeThrough(map, nx, ny, database)) {
            continue;
          }
          besideVisibleFloor = true;
          break;
        }
      }
      if (besideVisibleFloor) {
        markCellVisibleAndExplored(map, x, y);
      }
    }
  }
}

} // namespace

bool isTileEffectivelySeeThrough(const TileInstance& tile, const db::Database& database) {
  if (tile.tilesetName.empty()) {
    return true;
  }
  // Only use override when it was actually authored (optional has_value).
  if (tile.tileOverrides.has_value() && tile.tileOverrides->isSeeThroughOverride.has_value()) {
    return *tile.tileOverrides->isSeeThroughOverride;
  }
  const auto* meta = resolveTileMetadata(tile, database);
  if (!meta) {
    // Missing tileset/meta → see-through (matches walkable); WARN already emitted.
    return true;
  }
  return meta->isSeeThrough;
}

bool doesTileBlockSight(const TileInstance& tile, const db::Database& database) {
  return !isTileEffectivelySeeThrough(tile, database);
}

bool isDestinationSeeThrough(const MapInstance& map,
                             int x,
                             int y,
                             const db::Database& database) {
  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return true;
  }
  return isTileEffectivelySeeThrough(*tile, database);
}

void updateMapVisibilityFromPlayer(MapInstance& map,
                                   int playerX,
                                   int playerY,
                                   const db::Database& database) {
  clearAllVisible(map);

  const auto boxSize = kPlayerVisionBoxSize;
  // Cast to every cell in the vision octagon (Chebyshev bound + Manhattan corner cut).
  for (auto y = playerY - boxSize; y <= playerY + boxSize; y++) {
    for (auto x = playerX - boxSize; x <= playerX + boxSize; x++) {
      if (x == playerX && y == playerY) {
        continue;
      }
      if (!isInPlayerVisionRange(x - playerX, y - playerY, boxSize)) {
        continue;
      }
      castVisibilityRay(map, playerX, playerY, x, y, database);
    }
  }

  // Player's own tile is always visible (and explored).
  markCellVisibleAndExplored(map, playerX, playerY);

  // Fill opaque wall-face gaps beside visible open tiles.
  lightOpaqueWallsBesideVisibleFloors(map, playerX, playerY, boxSize, database);
}

ExploredMapMask captureExploredMask(const MapInstance& map) {
  auto mask = ExploredMapMask{};
  mask.width = map.width;
  mask.height = map.height;
  if (map.width <= 0 || map.height <= 0) {
    return mask;
  }

  const auto cellCount = static_cast<size_t>(map.width) * static_cast<size_t>(map.height);
  const auto byteCount = (cellCount + 7) / 8;
  mask.bits.resize(byteCount, 0);

  for (auto y = 0; y < map.height; y++) {
    for (auto x = 0; x < map.width; x++) {
      const auto index = tileIndex(map, x, y);
      if (index < 0) {
        continue;
      }
      auto explored = false;
      for (size_t li = 0; li < map.tiles.size(); li++) {
        const auto& layerTiles = map.tiles[li];
        if (index >= static_cast<int>(layerTiles.size())) {
          continue;
        }
        if (layerTiles[static_cast<size_t>(index)].isExplored) {
          explored = true;
          break;
        }
      }
      if (!explored) {
        continue;
      }
      const auto bitIndex = static_cast<size_t>(index);
      mask.bits[bitIndex / 8] =
          static_cast<uint8_t>(mask.bits[bitIndex / 8] | (1u << (bitIndex % 8)));
    }
  }
  return mask;
}

void applyExploredMask(MapInstance& map, const ExploredMapMask& mask) {
  if (mask.width != map.width || mask.height != map.height || map.width <= 0 ||
      map.height <= 0) {
    return;
  }
  const auto cellCount = static_cast<size_t>(map.width) * static_cast<size_t>(map.height);
  const auto byteCount = (cellCount + 7) / 8;
  if (mask.bits.size() < byteCount) {
    return;
  }

  for (auto y = 0; y < map.height; y++) {
    for (auto x = 0; x < map.width; x++) {
      const auto index = tileIndex(map, x, y);
      if (index < 0) {
        continue;
      }
      const auto bitIndex = static_cast<size_t>(index);
      const auto explored =
          (mask.bits[bitIndex / 8] & static_cast<uint8_t>(1u << (bitIndex % 8))) != 0;
      if (!explored) {
        continue;
      }
      for (size_t li = 0; li < map.tiles.size(); li++) {
        auto& layerTiles = map.tiles[li];
        if (index >= static_cast<int>(layerTiles.size())) {
          continue;
        }
        layerTiles[static_cast<size_t>(index)].isExplored = true;
      }
    }
  }
}

} // namespace model
