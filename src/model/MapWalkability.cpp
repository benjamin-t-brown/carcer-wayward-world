#include "model/MapWalkability.h"
#include "bmin/StringInterop.h"
#include "sdl2w/Logger.h"
#include <algorithm>

namespace model {
namespace {

void collectLayerKeys(const MapInstance& map, bmin::DynArray<int>& layerKeys) {
  auto& tilesMap =
      const_cast<bmin::Map<int, bmin::DynArray<TileInstance>>&>(map.tiles);
  for (auto it = tilesMap.begin(); it != tilesMap.end(); ++it) {
    layerKeys.pushBack(it->key);
  }
  std::sort(layerKeys.begin(), layerKeys.end());
}

int tileIndex(const MapInstance& map, int x, int y) {
  if (map.width <= 0) {
    return -1;
  }
  return y * map.width + x;
}

// Non-empty tile on map.tileLayerNumber at (x,y), or nullptr if empty/missing/OOB.
const TileInstance* tileAtCurrentLayer(const MapInstance& map, int x, int y) {
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return nullptr;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return nullptr;
  }

  auto& tilesMap =
      const_cast<bmin::Map<int, bmin::DynArray<TileInstance>>&>(map.tiles);
  if (!tilesMap.contains(map.tileLayerNumber)) {
    return nullptr;
  }
  const auto& layerTiles = tilesMap[map.tileLayerNumber];
  if (index >= static_cast<int>(layerTiles.size())) {
    return nullptr;
  }
  const auto& tile = layerTiles[static_cast<size_t>(index)];
  if (tile.tilesetName.empty()) {
    return nullptr;
  }
  return &tile;
}

TileInstance* tileAtCurrentLayer(MapInstance& map, int x, int y) {
  return const_cast<TileInstance*>(
      tileAtCurrentLayer(static_cast<const MapInstance&>(map), x, y));
}

} // namespace

const TileMetadata* findTileMetadata(const TilesetTemplate& tileset, int tileId) {
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

const TileMetadata* resolveTileMetadata(const TileInstance& tile,
                                        const db::Database& database) {
  if (tile.tilesetName.empty()) {
    return nullptr;
  }
  const auto* tileset = database.findTilesetTemplate(bmin::toStringView(tile.tilesetName));
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

bool isTileEffectivelyWalkable(const TileInstance& tile, const db::Database& database) {
  if (tile.tilesetName.empty()) {
    return true;
  }
  // Only use override when it was actually authored (optional has_value).
  if (tile.tileOverrides.has_value() && tile.tileOverrides->isWalkableOverride.has_value()) {
    return *tile.tileOverrides->isWalkableOverride;
  }
  const auto* meta = resolveTileMetadata(tile, database);
  if (!meta) {
    // Missing tileset/meta → walkable (matches ceditor); WARN already emitted.
    return true;
  }
  return meta->isWalkable;
}

bool isClosedDoorTile(const TileInstance& tile, const db::Database& database) {
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

void collectTilesAt(MapInstance& map, int x, int y, bmin::DynArray<TileInstance*>& out) {
  out.clear();
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return;
  }

  auto layerKeys = bmin::DynArray<int>{};
  collectLayerKeys(map, layerKeys);

  for (size_t li = 0; li < layerKeys.size(); li++) {
    auto layerKey = layerKeys[li];
    if (!map.tiles.contains(layerKey)) {
      continue;
    }
    auto& layerTiles = map.tiles[layerKey];
    if (index >= static_cast<int>(layerTiles.size())) {
      continue;
    }
    auto& tile = layerTiles[static_cast<size_t>(index)];
    if (tile.tilesetName.empty()) {
      continue;
    }
    out.pushBack(&tile);
  }
}

void collectTilesAt(const MapInstance& map,
                    int x,
                    int y,
                    bmin::DynArray<const TileInstance*>& out) {
  out.clear();
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return;
  }

  auto layerKeys = bmin::DynArray<int>{};
  collectLayerKeys(map, layerKeys);

  auto& tilesMap =
      const_cast<bmin::Map<int, bmin::DynArray<TileInstance>>&>(map.tiles);
  for (size_t li = 0; li < layerKeys.size(); li++) {
    auto layerKey = layerKeys[li];
    if (!tilesMap.contains(layerKey)) {
      continue;
    }
    const auto& layerTiles = tilesMap[layerKey];
    if (index >= static_cast<int>(layerTiles.size())) {
      continue;
    }
    const auto& tile = layerTiles[static_cast<size_t>(index)];
    if (tile.tilesetName.empty()) {
      continue;
    }
    out.pushBack(&tile);
  }
}

const TileInstance* resolveTileToRender(const MapInstance& map, int x, int y) {
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return nullptr;
  }
  const auto index = tileIndex(map, x, y);
  if (index < 0) {
    return nullptr;
  }

  auto layerKeys = bmin::DynArray<int>{};
  collectLayerKeys(map, layerKeys);

  auto& tilesMap =
      const_cast<bmin::Map<int, bmin::DynArray<TileInstance>>&>(map.tiles);
  const TileInstance* best = nullptr;
  for (size_t li = 0; li < layerKeys.size(); li++) {
    auto layerKey = layerKeys[li];
    if (layerKey > map.tileLayerNumber) {
      break;
    }
    if (!tilesMap.contains(layerKey)) {
      continue;
    }
    const auto& layerTiles = tilesMap[layerKey];
    if (index >= static_cast<int>(layerTiles.size())) {
      continue;
    }
    const auto& tile = layerTiles[static_cast<size_t>(index)];
    if (tile.tilesetName.empty()) {
      continue;
    }
    best = &tile;
  }
  return best;
}

bool isDestinationWalkable(const MapInstance& map,
                           int x,
                           int y,
                           const db::Database& database) {
  const auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return true;
  }
  return isTileEffectivelyWalkable(*tile, database);
}

TileInstance* findClosedDoorAt(MapInstance& map,
                               int x,
                               int y,
                               const db::Database& database) {
  auto* tile = tileAtCurrentLayer(map, x, y);
  if (!tile) {
    return nullptr;
  }
  if (isClosedDoorTile(*tile, database)) {
    return tile;
  }
  return nullptr;
}

} // namespace model
