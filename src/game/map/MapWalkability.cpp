#include "game/map/MapWalkability.h"
#include "bmin/StringInterop.h"
#include "sdl2w/Logger.h"

namespace game {
namespace {

int tileIndex(const model::MapInstance& map, int x, int y) {
  if (map.width <= 0) {
    return -1;
  }
  return y * map.width + x;
}

} // namespace

// Non-empty tile on map.tileLayerNumber at (x,y), or nullptr if empty/missing/OOB.
const model::TileInstance*
tileAtCurrentLayer(const model::MapInstance& map, int x, int y) {
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return nullptr;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return nullptr;
  }

  const auto* layerTiles =
      model::mapLayerPtr(model::mapInstanceTiles(map), map.tileLayerNumber);
  if (!layerTiles) {
    return nullptr;
  }
  if (index >= static_cast<int>(layerTiles->size())) {
    return nullptr;
  }
  const auto& tile = (*layerTiles)[static_cast<size_t>(index)];
  if (tile.tilesetName.empty()) {
    return nullptr;
  }
  return &tile;
}

model::TileInstance* tileAtCurrentLayer(model::MapInstance& map, int x, int y) {
  return const_cast<model::TileInstance*>(
      tileAtCurrentLayer(static_cast<const model::MapInstance&>(map), x, y));
}

const model::TileMetadata* findTileMetadata(const model::TilesetTemplate& tileset,
                                            int tileId) {
  if (tileId >= 0 && tileId < static_cast<int>(tileset.tiles.size())) {
    const auto& meta = tileset.tiles[static_cast<size_t>(tileId)];
    if (meta.id == tileId) {
      return &meta;
    }
  }
  for (size_t i = 0; i < tileset.tiles.size(); i++) {
    if (tileset.tiles[i].id == tileId) {
      return &tileset.tiles[i];
    }
  }
  return nullptr;
}

const model::TileMetadata* resolveTileMetadata(const model::TileInstance& tile,
                                               const db::Database& database) {
  if (tile.tilesetName.empty()) {
    return nullptr;
  }
  const auto* tileset =
      database.findTilesetTemplate(bmin::toStringView(tile.tilesetName));
  if (!tileset) {
    LOG(WARN) << "resolveTileMetadata: tileset not found: " << tile.tilesetName
              << LOG_ENDL;
    return nullptr;
  }
  const auto* meta = findTileMetadata(*tileset, tile.tileId);
  if (!meta) {
    LOG(WARN) << "resolveTileMetadata: tile metadata not found: tileset="
              << tile.tilesetName << " tileId=" << tile.tileId << LOG_ENDL;
  }
  return meta;
}

bool isTileEffectivelyWalkable(const model::TileInstance& tile,
                               const db::Database& database) {
  if (tile.tilesetName.empty()) {
    return true;
  }
  // Only use override when it was actually authored (optional has_value).
  if (tile.tileOverrides.has_value() &&
      tile.tileOverrides->isWalkableOverride.has_value()) {
    return *tile.tileOverrides->isWalkableOverride;
  }
  const auto* meta = resolveTileMetadata(tile, database);
  if (!meta) {
    // Missing tileset/meta → walkable (matches ceditor); WARN already emitted.
    return true;
  }
  return meta->isWalkable;
}

bool isClosedDoorTile(const model::TileInstance& tile, const db::Database& database) {
  if (tile.tilesetName.empty()) {
    return false;
  }
  const auto* meta = resolveTileMetadata(tile, database);
  if (!meta || !meta->isDoor) {
    return false;
  }
  // Closed vs open from tileset metadata only — map walkability overrides must
  // not reclassify an open door as closed (or skip opening a closed door).
  return !meta->isWalkable;
}

bool isOpenDoorTile(const model::TileInstance& tile, const db::Database& database) {
  if (tile.tilesetName.empty()) {
    return false;
  }
  const auto* meta = resolveTileMetadata(tile, database);
  if (!meta || !meta->isDoor) {
    return false;
  }
  return meta->isWalkable;
}

bmin::DynArray<model::OpenedDoorRecord> captureOpenedDoors(const model::MapInstance& map,
                                                           const db::Database& database) {
  auto doors = bmin::DynArray<model::OpenedDoorRecord>{};
  if (map.width <= 0 || map.height <= 0) {
    return doors;
  }

  auto& tiles = const_cast<model::TileLayerMap&>(model::mapInstanceTiles(map));
  for (auto it = tiles.begin(); it != tiles.end(); ++it) {
    const auto& layerTiles = it->value;
    for (size_t ti = 0; ti < layerTiles.size(); ti++) {
      const auto& tile = layerTiles[ti];
      if (!isOpenDoorTile(tile, database)) {
        continue;
      }
      auto record = model::OpenedDoorRecord{};
      record.layer = it->key;
      record.x = tile.x;
      record.y = tile.y;
      record.tileId = tile.tileId;
      doors.pushBack(record);
    }
  }
  return doors;
}

void applyOpenedDoors(model::MapInstance& map,
                      const bmin::DynArray<model::OpenedDoorRecord>& doors) {
  for (size_t i = 0; i < doors.size(); i++) {
    const auto& record = doors[i];
    if (record.x < 0 || record.y < 0 || record.x >= map.width || record.y >= map.height) {
      continue;
    }
    auto* layerTiles = model::mapLayerPtr(model::mapInstanceTiles(map), record.layer);
    if (!layerTiles) {
      continue;
    }
    const auto index = tileIndex(map, record.x, record.y);
    if (index < 0 || index >= static_cast<int>(layerTiles->size())) {
      continue;
    }
    auto& tile = (*layerTiles)[static_cast<size_t>(index)];
    if (tile.tilesetName.empty()) {
      continue;
    }
    tile.tileId = record.tileId;
  }
}

void collectTilesAt(model::MapInstance& map,
                    int x,
                    int y,
                    bmin::DynArray<model::TileInstance*>& out) {
  out.clear();
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return;
  }

  auto minMax = model::mapInstanceGetMinMaxLayer(map);
  for (int layerKey = minMax.x; layerKey <= minMax.y; ++layerKey) {
    auto* layerTiles = model::mapLayerPtr(model::mapInstanceTiles(map), layerKey);
    if (!layerTiles || index >= static_cast<int>(layerTiles->size())) {
      continue;
    }
    auto& tile = (*layerTiles)[static_cast<size_t>(index)];
    if (tile.tilesetName.empty()) {
      continue;
    }
    out.pushBack(&tile);
  }
}

void collectTilesAt(const model::MapInstance& map,
                    int x,
                    int y,
                    bmin::DynArray<const model::TileInstance*>& out) {
  out.clear();
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return;
  }

  auto minMax = model::mapInstanceGetMinMaxLayer(map);
  for (int layerKey = minMax.x; layerKey <= minMax.y; ++layerKey) {
    const auto* layerTiles = model::mapLayerPtr(model::mapInstanceTiles(map), layerKey);
    if (!layerTiles || index >= static_cast<int>(layerTiles->size())) {
      continue;
    }
    const auto& tile = (*layerTiles)[static_cast<size_t>(index)];
    if (tile.tilesetName.empty()) {
      continue;
    }
    out.pushBack(&tile);
  }
}

const model::TileInstance*
resolveTileToRender(const model::MapInstance& map, int x, int y) {
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return nullptr;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return nullptr;
  }

  const model::TileInstance* best = nullptr;
  const int maxLayer = map.tileLayerNumber;
  for (int layerKey = 0; layerKey <= maxLayer; ++layerKey) {
    const auto* layerTiles = model::mapLayerPtr(model::mapInstanceTiles(map), layerKey);
    if (!layerTiles) {
      continue;
    }
    if (index >= static_cast<int>(layerTiles->size())) {
      continue;
    }
    const auto& tile = (*layerTiles)[static_cast<size_t>(index)];
    if (tile.tilesetName.empty()) {
      continue;
    }
    best = &tile;
  }
  return best;
}

bool isTileCurrentlyVisible(const model::MapInstance& map, int x, int y) {
  const auto* tile = resolveTileToRender(map, x, y);
  return tile != nullptr && tile->isVisible;
}

bool isDestinationWalkable(const model::MapInstance& map,
                           int x,
                           int y,
                           const db::Database& database) {
  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return true;
  }
  return isTileEffectivelyWalkable(*tile, database);
}

model::TileInstance*
findClosedDoorAt(model::MapInstance& map, int x, int y, const db::Database& database) {
  auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return nullptr;
  }
  if (isClosedDoorTile(*tile, database)) {
    return tile;
  }
  return nullptr;
}

} // namespace game
