#include "game/map/MapVision.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapWalkability.h"
#include "model/Combat.h"
#include "model/instances/Player.h"
#include <cstdint>
#include <cstdlib>


namespace game {
namespace {

int tileIndex(const model::MapInstance& map, int x, int y) {
  if (map.width <= 0) {
    return -1;
  }
  return y * map.width + x;
}

bool inBounds(const model::MapInstance& map, int x, int y) {
  return x >= 0 && y >= 0 && x < map.width && y < map.height;
}

void clearAllVisible(model::MapInstance& map) {
  for (auto it = model::mapInstanceTiles(map).begin();
       it != model::mapInstanceTiles(map).end();
       ++it) {
    auto& layerTiles = it->value;
    for (size_t ti = 0; ti < layerTiles.size(); ti++) {
      layerTiles[ti].isVisible = false;
    }
  }
}

// Mark isVisible / isExplored on the active map.tileLayerNumber cell only
// (including empty tilesetName cells — holes still carry fog flags).
void markCellVisibleAndExplored(model::MapInstance& map, int x, int y) {
  if (!inBounds(map, x, y)) {
    return;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return;
  }

  auto* layerTiles =
      model::mapLayerPtr(model::mapInstanceTiles(map), map.tileLayerNumber);
  if (!layerTiles || index >= static_cast<int>(layerTiles->size())) {
    return;
  }
  auto& tile = (*layerTiles)[static_cast<size_t>(index)];
  tile.isVisible = true;
  tile.isExplored = true;
}

void castVisibilityRay(model::MapInstance& map,
                       int x1,
                       int y1,
                       int x2,
                       int y2,
                       const db::Database& database) {
  auto visibility = true;
  // Standing on an opaque tile must not block the observer's own rays.
  const auto originX = x1;
  const auto originY = y1;

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

    const auto atOrigin = x1 == originX && y1 == originY;
    if (!atOrigin && !isDestinationSeeThrough(map, x1, y1, database)) {
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

// True if Bresenham from (ox,oy) to (tx,ty) hits no opaque cell before the target.
// The target itself may be opaque (first-opaque-on-ray rule for wall-face fill).
bool approachClearOfOpaque(model::MapInstance& map,
                           int ox,
                           int oy,
                           int tx,
                           int ty,
                           const db::Database& database) {
  const auto dx = std::abs(tx - ox);
  const auto sx = ox < tx ? 1 : -1;
  const auto dy = std::abs(ty - oy);
  const auto sy = oy < ty ? 1 : -1;
  auto err = (dx > dy ? dx : -dy) / 2;
  auto x = ox;
  auto y = oy;

  while (true) {
    if (!inBounds(map, x, y)) {
      return false;
    }
    const auto atOrigin = x == ox && y == oy;
    const auto atTarget = x == tx && y == ty;
    if (!atOrigin && !atTarget && !isDestinationSeeThrough(map, x, y, database)) {
      return false;
    }
    if (atTarget) {
      return true;
    }

    const auto e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x += sx;
    }
    if (e2 < dy) {
      err += dx;
      y += sy;
    }
  }
}

// Perimeter Bresenham leaves gaps along opaque wall faces (a ray that hits one wall
// tile stops, so neighbors on the same wall never get their own hit). Light opaque
// cells that touch a visible see-through cell, but only if they are the first opaque
// on the approach from the observer (no nearer wall between).
void lightOpaqueWallsBesideVisibleFloors(model::MapInstance& map,
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
      if (const auto* layerTiles =
              model::mapLayerPtr(model::mapInstanceTiles(map), map.tileLayerNumber)) {
        if (index < static_cast<int>(layerTiles->size())) {
          alreadyVisible = (*layerTiles)[static_cast<size_t>(index)].isVisible;
        }
      }
      if (alreadyVisible) {
        continue;
      }
      if (isDestinationSeeThrough(map, x, y, database)) {
        continue;
      }
      if (!approachClearOfOpaque(map, playerX, playerY, x, y, database)) {
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
          const auto* layerTiles =
              model::mapLayerPtr(model::mapInstanceTiles(map), map.tileLayerNumber);
          if (nIndex < 0 || !layerTiles ||
              nIndex >= static_cast<int>(layerTiles->size())) {
            continue;
          }
          if (!(*layerTiles)[static_cast<size_t>(nIndex)].isVisible) {
            continue;
          }
          // Observer standing tile counts as see-through for wall-face lighting.
          const auto atObserver = nx == playerX && ny == playerY;
          if (!atObserver && !isDestinationSeeThrough(map, nx, ny, database)) {
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

void addMapVisibilityFromPoint(model::MapInstance& map,
                               int playerX,
                               int playerY,
                               const db::Database& database) {
  const auto boxSize = kPlayerVisionBoxSize;
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

  markCellVisibleAndExplored(map, playerX, playerY);
  lightOpaqueWallsBesideVisibleFloors(map, playerX, playerY, boxSize, database);
}

bool inWorldBounds(ActiveMapOrchestrator& orch, int worldX, int worldY) {
  const auto total = orch.getTotalMapTilesSize();
  return total.valid && worldX >= 0 && worldY >= 0 && worldX < total.x && worldY < total.y;
}

void clearAllVisibleInActiveGrid(ActiveMapOrchestrator& orch, int mapLayer) {
  const auto& grid = orch.getMapGrid();
  for (int gy = 0; gy < grid.gridHeight; ++gy) {
    for (int gx = 0; gx < grid.gridWidth; ++gx) {
      const auto& mapName = grid.cells[static_cast<size_t>(gy)][static_cast<size_t>(gx)];
      if (mapName.empty()) {
        continue;
      }
      if (auto* map = orch.getMapInstanceAt(gx * grid.mapWidth, gy * grid.mapHeight)) {
        clearAllVisible(*map);
        map->tileLayerNumber = mapLayer;
      }
    }
  }
}

void markWorldCellVisibleAndExplored(ActiveMapOrchestrator& orch,
                                     int worldX,
                                     int worldY,
                                     int mapLayer) {
  auto* map = orch.getMapInstanceAt(worldX, worldY);
  const auto local = orch.activeMapCoordToInstanceCoord(worldX, worldY);
  if (!map || !local.valid) {
    return;
  }
  map->tileLayerNumber = mapLayer;
  markCellVisibleAndExplored(*map, local.x, local.y);
}

bool isWorldCellSeeThrough(ActiveMapOrchestrator& orch,
                           int worldX,
                           int worldY,
                           int mapLayer,
                           const db::Database& database) {
  auto* map = orch.getMapInstanceAt(worldX, worldY);
  const auto local = orch.activeMapCoordToInstanceCoord(worldX, worldY);
  if (!map || !local.valid) {
    return true;
  }
  const auto* tile = model::mapInstanceGetTileAt(*map, local.x, local.y, mapLayer);
  if (!tile || tile->tilesetName.empty()) {
    return true;
  }
  return isTileEffectivelySeeThrough(*tile, database);
}

bool isWorldCellVisible(ActiveMapOrchestrator& orch,
                        int worldX,
                        int worldY,
                        int mapLayer) {
  auto* map = orch.getMapInstanceAt(worldX, worldY);
  const auto local = orch.activeMapCoordToInstanceCoord(worldX, worldY);
  if (!map || !local.valid || map->width <= 0) {
    return false;
  }
  const auto index = tileIndex(*map, local.x, local.y);
  const auto* layerTiles =
      model::mapLayerPtr(model::mapInstanceTiles(*map), mapLayer);
  if (index < 0 || !layerTiles || index >= static_cast<int>(layerTiles->size())) {
    return false;
  }
  return (*layerTiles)[static_cast<size_t>(index)].isVisible;
}

void castWorldVisibilityRay(ActiveMapOrchestrator& orch,
                            int x1,
                            int y1,
                            int x2,
                            int y2,
                            int mapLayer,
                            const db::Database& database) {
  auto visibility = true;
  // Standing on an opaque tile must not block the observer's own rays.
  const auto originX = x1;
  const auto originY = y1;

  const auto dx = std::abs(x2 - x1);
  const auto sx = x1 < x2 ? 1 : -1;
  const auto dy = std::abs(y2 - y1);
  const auto sy = y1 < y2 ? 1 : -1;
  auto err = (dx > dy ? dx : -dy) / 2;

  while (true) {
    if (!inWorldBounds(orch, x1, y1)) {
      break;
    }

    if (visibility) {
      markWorldCellVisibleAndExplored(orch, x1, y1, mapLayer);
    }

    const auto atOrigin = x1 == originX && y1 == originY;
    if (!atOrigin && !isWorldCellSeeThrough(orch, x1, y1, mapLayer, database)) {
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

bool approachClearOfOpaqueWorld(ActiveMapOrchestrator& orch,
                                int ox,
                                int oy,
                                int tx,
                                int ty,
                                int mapLayer,
                                const db::Database& database) {
  const auto dx = std::abs(tx - ox);
  const auto sx = ox < tx ? 1 : -1;
  const auto dy = std::abs(ty - oy);
  const auto sy = oy < ty ? 1 : -1;
  auto err = (dx > dy ? dx : -dy) / 2;
  auto x = ox;
  auto y = oy;

  while (true) {
    if (!inWorldBounds(orch, x, y)) {
      return false;
    }
    const auto atOrigin = x == ox && y == oy;
    const auto atTarget = x == tx && y == ty;
    if (!atOrigin && !atTarget &&
        !isWorldCellSeeThrough(orch, x, y, mapLayer, database)) {
      return false;
    }
    if (atTarget) {
      return true;
    }

    const auto e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x += sx;
    }
    if (e2 < dy) {
      err += dx;
      y += sy;
    }
  }
}

void lightOpaqueWallsBesideVisibleFloorsWorld(ActiveMapOrchestrator& orch,
                                              int playerX,
                                              int playerY,
                                              int boxSize,
                                              int mapLayer,
                                              const db::Database& database) {
  for (auto y = playerY - boxSize; y <= playerY + boxSize; y++) {
    for (auto x = playerX - boxSize; x <= playerX + boxSize; x++) {
      if (!isInPlayerVisionRange(x - playerX, y - playerY, boxSize)) {
        continue;
      }
      if (!inWorldBounds(orch, x, y)) {
        continue;
      }
      if (isWorldCellVisible(orch, x, y, mapLayer)) {
        continue;
      }
      if (isWorldCellSeeThrough(orch, x, y, mapLayer, database)) {
        continue;
      }
      if (!approachClearOfOpaqueWorld(
              orch, playerX, playerY, x, y, mapLayer, database)) {
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
          if (!inWorldBounds(orch, nx, ny)) {
            continue;
          }
          if (!isWorldCellVisible(orch, nx, ny, mapLayer)) {
            continue;
          }
          // Observer standing tile counts as see-through for wall-face lighting.
          const auto atObserver = nx == playerX && ny == playerY;
          if (!atObserver &&
              !isWorldCellSeeThrough(orch, nx, ny, mapLayer, database)) {
            continue;
          }
          besideVisibleFloor = true;
          break;
        }
      }
      if (besideVisibleFloor) {
        markWorldCellVisibleAndExplored(orch, x, y, mapLayer);
      }
    }
  }
}

void addActiveMapVisibilityFromPoint(ActiveMapOrchestrator& orch,
                                     int worldX,
                                     int worldY,
                                     int mapLayer,
                                     const db::Database& database) {
  const auto boxSize = kPlayerVisionBoxSize;
  for (auto y = worldY - boxSize; y <= worldY + boxSize; y++) {
    for (auto x = worldX - boxSize; x <= worldX + boxSize; x++) {
      if (x == worldX && y == worldY) {
        continue;
      }
      if (!isInPlayerVisionRange(x - worldX, y - worldY, boxSize)) {
        continue;
      }
      castWorldVisibilityRay(orch, worldX, worldY, x, y, mapLayer, database);
    }
  }

  markWorldCellVisibleAndExplored(orch, worldX, worldY, mapLayer);
  lightOpaqueWallsBesideVisibleFloorsWorld(
      orch, worldX, worldY, boxSize, mapLayer, database);
}

} // namespace

bool isTileEffectivelySeeThrough(const model::TileInstance& tile,
                                 const db::Database& database) {
  if (tile.tilesetName.empty()) {
    return true;
  }
  // Only use override when it was actually authored (optional has_value).
  if (tile.tileOverrides.has_value() &&
      tile.tileOverrides->isSeeThroughOverride.has_value()) {
    return *tile.tileOverrides->isSeeThroughOverride;
  }
  const auto* meta = resolveTileMetadata(tile, database);
  if (!meta) {
    // Missing tileset/meta → see-through (matches walkable); WARN already emitted.
    return true;
  }
  return meta->isSeeThrough;
}

bool doesTileBlockSight(const model::TileInstance& tile, const db::Database& database) {
  return !isTileEffectivelySeeThrough(tile, database);
}

bool isDestinationSeeThrough(const model::MapInstance& map,
                             int x,
                             int y,
                             const db::Database& database) {
  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return true;
  }
  return isTileEffectivelySeeThrough(*tile, database);
}

void updateMapVisibilityFromPlayer(model::MapInstance& map,
                                   int playerX,
                                   int playerY,
                                   const db::Database& database) {
  clearAllVisible(map);
  addMapVisibilityFromPoint(map, playerX, playerY, database);
}

void updateMapVisibilityFromParty(model::MapInstance& map,
                                  const model::Player& player,
                                  const db::Database& database) {
  clearAllVisible(map);
  for (const auto& character : map.persistentState.characters) {
    if (model::isPartyMember(player, character.id)) {
      addMapVisibilityFromPoint(map, character.x, character.y, database);
    }
  }
}

void updateActiveMapVisibilityFromParty(model::World& world,
                                        MapInstanceStore& mapInstances,
                                        const model::Player& player,
                                        const db::Database& database) {
  if (world.activeMap.gridId.empty()) {
    return;
  }
  ActiveMapOrchestrator orch(world.activeMap, mapInstances, &database);
  clearAllVisibleInActiveGrid(orch, world.activeMap.mapLayer);

  for (const auto& character : world.activeMap.characters) {
    if (!model::isPartyMember(player, character.id)) {
      continue;
    }
    addActiveMapVisibilityFromPoint(
        orch, character.x, character.y, world.activeMap.mapLayer, database);
  }
}

void updateActiveMapVisibilityFromPlayer(model::World& world,
                                         MapInstanceStore& mapInstances,
                                         int worldX,
                                         int worldY,
                                         const db::Database& database) {
  if (world.activeMap.gridId.empty()) {
    return;
  }
  ActiveMapOrchestrator orch(world.activeMap, mapInstances, &database);
  if (!inWorldBounds(orch, worldX, worldY)) {
    return;
  }
  clearAllVisibleInActiveGrid(orch, world.activeMap.mapLayer);
  addActiveMapVisibilityFromPoint(
      orch, worldX, worldY, world.activeMap.mapLayer, database);
}

model::ExploredMapMask captureExploredMask(const model::MapInstance& map) {
  auto mask = model::ExploredMapMask{};
  mask.width = map.width;
  mask.height = map.height;
  if (map.width <= 0 || map.height <= 0) {
    return mask;
  }

  const auto cellCount = static_cast<size_t>(map.width) * static_cast<size_t>(map.height);
  const auto byteCount = (cellCount + 7) / 8;
  const auto& tiles = model::mapInstanceTiles(map);
  for (auto it = tiles.begin(); it != tiles.end(); ++it) {
    const auto layerKey = it->key;
    const auto& layerTiles = it->value;
    auto bits = bmin::DynArray<uint8_t>{};
    bits.resize(byteCount, 0);
    auto anyExplored = false;
    for (auto y = 0; y < map.height; y++) {
      for (auto x = 0; x < map.width; x++) {
        const auto index = tileIndex(map, x, y);
        if (index < 0 || index >= static_cast<int>(layerTiles.size())) {
          continue;
        }
        if (!layerTiles[static_cast<size_t>(index)].isExplored) {
          continue;
        }
        anyExplored = true;
        const auto bitIndex = static_cast<size_t>(index);
        bits[bitIndex / 8] =
            static_cast<uint8_t>(bits[bitIndex / 8] | (1u << (bitIndex % 8)));
      }
    }
    if (anyExplored) {
      mask.layers[layerKey] = std::move(bits);
    }
  }
  return mask;
}

void applyExploredMask(model::MapInstance& map, const model::ExploredMapMask& mask) {
  if (mask.width != map.width || mask.height != map.height || map.width <= 0 ||
      map.height <= 0) {
    return;
  }
  const auto cellCount = static_cast<size_t>(map.width) * static_cast<size_t>(map.height);
  const auto byteCount = (cellCount + 7) / 8;

  for (auto it = mask.layers.begin(); it != mask.layers.end(); ++it) {
    const auto layerKey = it->key;
    const auto& bits = it->value;
    if (bits.size() < byteCount) {
      continue;
    }
    auto* layerTiles = model::mapLayerPtr(model::mapInstanceTiles(map), layerKey);
    if (!layerTiles) {
      continue;
    }
    for (auto y = 0; y < map.height; y++) {
      for (auto x = 0; x < map.width; x++) {
        const auto index = tileIndex(map, x, y);
        if (index < 0 || index >= static_cast<int>(layerTiles->size())) {
          continue;
        }
        const auto bitIndex = static_cast<size_t>(index);
        const auto explored =
            (bits[bitIndex / 8] & static_cast<uint8_t>(1u << (bitIndex % 8))) != 0;
        if (!explored) {
          continue;
        }
        (*layerTiles)[static_cast<size_t>(index)].isExplored = true;
      }
    }
  }
}

} // namespace game
