#pragma once

#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/ItemInstance.h"
#include "model/instances/TileInstance.h"
#include "model/templates/Maps.h"
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
  // bmin::DynArray<game::TileField> fields;
};

struct PersistentMapState {
  int version = 2;
  ExploredMapMask explored;
  bmin::DynArray<OpenedDoorRecord> openedDoors;
  // bmin::DynArray<DefeatedCharacterRecord> defeatedCharacters;
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

inline TileInstance* mapInstanceGetTileAt(MapInstance& map, int x, int y, int layer) {
  if (!map.tiles.contains(layer)) {
    return nullptr;
  }
  auto layerTiles = map.tiles[layer];
  size_t index = static_cast<size_t>(y * map.width + x);
  if (index >= layerTiles.size()) {
    return nullptr;
  }
  return &layerTiles[index];
}

inline TileXY mapInstanceGetMinMaxLayer(MapInstance& map) {
  TileXY minMaxLayer = {0, 0};
  for (auto it = map.tiles.begin(); it != map.tiles.end(); ++it) {
    minMaxLayer.x = std::min(minMaxLayer.x, it->key);
    minMaxLayer.y = std::max(minMaxLayer.y, it->key);
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

// CharacterInstance* findCharacterAt(MapInstance& map,
//                                    int x,
//                                    int y,
//                                    const bmin::String& excludeId = bmin::String{});

} // namespace model
