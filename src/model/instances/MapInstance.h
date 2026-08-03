#pragma once

#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/ItemInstance.h"
#include "model/instances/TileInstance.h"
#include "model/templates/Maps.h"
#include <algorithm>
#include <cstdint>

namespace model {

enum class TurnMode { TURN_TOWN, TURN_OUTDOOR, TURN_COMBAT };

// Session-scoped fog-of-war memory for a map template (one bit per cell).
struct ExploredMapMask {
  int width = 0;
  int height = 0;
  bmin::DynArray<uint8_t> bits;
};

// Open-door tileId mutation on a map (closed doors become tileId+1 at runtime).
struct OpenedDoorRecord {
  int layer = 0;
  int x = 0;
  int y = 0;
  int tileId = 0;
};

// Map-placed character removed for the session (matched on template + spawn tile).
struct DefeatedCharacterRecord {
  bmin::String templateName;
  int x = 0;
  int y = 0;
};

// Tile overlay fields persisted per layer/cell.
struct PersistentTileFieldRecord {
  int layer = 0;
  int x = 0;
  int y = 0;
  bmin::DynArray<game::TileField> fields;
};

struct PersistentMapState {
  int version = 2;
  ExploredMapMask explored;
  bmin::DynArray<OpenedDoorRecord> openedDoors;
  bmin::DynArray<DefeatedCharacterRecord> defeatedCharacters;
  bmin::DynArray<PersistentTileFieldRecord> tileFields;

  bmin::Map<int, bmin::DynArray<TileInstance>> tiles;
  bmin::DynArray<CharacterInstance> characters;
  bmin::DynArray<ItemInstance> items;
};

struct MapInstance {
  bmin::String id;
  bmin::String label;
  bmin::String templateName;

  PersistentMapState persistentState;
  int width = 0;
  int height = 0;
  int spriteWidth = 0;
  int spriteHeight = 0;
  int tileLayerNumber = 0;
  MapType mapType = MapType::TOWN;
};

struct TileXY {
  int x = 0;
  int y = 0;
};

using TileLayerMap = bmin::Map<int, bmin::DynArray<TileInstance>>;

inline TileLayerMap& mapInstanceTiles(MapInstance& map) { return map.persistentState.tiles; }

inline const TileLayerMap& mapInstanceTiles(const MapInstance& map) {
  return map.persistentState.tiles;
}

inline bool mapHasLayer(const TileLayerMap& layers, int layer) {
  return layers.contains(layer);
}

inline bool mapInstanceHasLayer(const TileLayerMap& layers, int layer) {
  return mapHasLayer(layers, layer);
}

inline bmin::DynArray<TileInstance>& mapLayerAt(TileLayerMap& layers, int layer) {
  if (!layers.contains(layer)) {
    layers[layer] = bmin::DynArray<TileInstance>{};
  }
  return layers[layer];
}

inline const bmin::DynArray<TileInstance>* mapLayerPtr(const TileLayerMap& layers, int layer) {
  if (!layers.contains(layer)) {
    return nullptr;
  }
  auto& mutableLayers = const_cast<TileLayerMap&>(layers);
  return &mutableLayers[layer];
}

inline bmin::DynArray<TileInstance>* mapLayerPtr(TileLayerMap& layers, int layer) {
  if (!layers.contains(layer)) {
    return nullptr;
  }
  return &layers[layer];
}

inline TileInstance* mapInstanceGetTileAt(MapInstance& map, int x, int y, int layer) {
  auto* layerTiles = mapLayerPtr(mapInstanceTiles(map), layer);
  if (!layerTiles || map.width <= 0) {
    return nullptr;
  }
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return nullptr;
  }
  size_t index = static_cast<size_t>(y * map.width + x);
  if (index >= layerTiles->size()) {
    return nullptr;
  }
  return &(*layerTiles)[index];
}

inline const TileInstance* mapInstanceGetTileAt(const MapInstance& map,
                                                int x,
                                                int y,
                                                int layer) {
  return mapInstanceGetTileAt(const_cast<MapInstance&>(map), x, y, layer);
}

inline TileXY mapInstanceGetMinMaxLayer(const MapInstance& map) {
  TileXY minMaxLayer = {0, 0};
  bool first = true;
  auto& tiles = const_cast<TileLayerMap&>(mapInstanceTiles(map));
  for (auto it = tiles.begin(); it != tiles.end(); ++it) {
    if (first) {
      minMaxLayer.x = it->key;
      minMaxLayer.y = it->key;
      first = false;
    } else {
      minMaxLayer.x = std::min(minMaxLayer.x, it->key);
      minMaxLayer.y = std::max(minMaxLayer.y, it->key);
    }
  }
  return minMaxLayer;
}

MapInstance createMapInstanceFromTemplate(const CarcerMapTemplate& mapTemplate);

// Flat cell index → tile (x, y); matches createMapInstanceFromTemplate math.
TileXY tileIndexToXY(int i, int width);
int tileXYToIndex(int x, int y, int width);

// First marker whose name matches (ceditor findMarkerOnMap semantics).
const MapMarkerPlacement* findMarkerOnTemplate(const CarcerMapTemplate& mapTemplate,
                                               const bmin::String& markerName);

CharacterInstance* mapInstanceFindCharacter(MapInstance& map, const bmin::String& id);
const CharacterInstance* mapInstanceFindCharacter(const MapInstance& map,
                                                  const bmin::String& id);

} // namespace model
