#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/instances/ItemInstance.h"
#include "model/instances/TileInstance.h"
#include "model/templates/Maps.h"
#include <cstdint>
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

enum class TurnMode { TURN_TOWN, TURN_OUTDOOR, TURN_COMBAT };

struct MapInstance {
  bmin::String id;
  bmin::String label;
  bmin::String templateName;
  bmin::DynArray<bmin::DynArray<TileInstance>> tiles;
  bmin::DynArray<CharacterInstance> characters;
  bmin::DynArray<ItemInstance> items;
  int width = 0;
  int height = 0;
  int spriteWidth = 0;
  int spriteHeight = 0;
  int tileLayerNumber = 0;
  MapType mapType = MapType::TOWN;
  TurnMode turnMode = TurnMode::TURN_TOWN;
};

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

// Per-map session (and future disk) deltas keyed by template name — not by grid.
struct PersistentMapState {
  int version = 2;
  ExploredMapMask explored;
  bmin::DynArray<OpenedDoorRecord> openedDoors;
  bmin::DynArray<DefeatedCharacterRecord> defeatedCharacters;
  bmin::DynArray<PersistentTileFieldRecord> tileFields;
};

struct TileXY {
  int x = 0;
  int y = 0;
};

inline bool mapHasLayer(const bmin::DynArray<bmin::DynArray<TileInstance>>& layers,
                        int layer) {
  return layer >= 0 && static_cast<size_t>(layer) < layers.size();
}

inline bmin::DynArray<TileInstance>& mapLayerAt(
    bmin::DynArray<bmin::DynArray<TileInstance>>& layers, int layer) {
  const auto idx = static_cast<size_t>(layer);
  if (layers.size() <= idx) {
    layers.resize(idx + 1);
  }
  return layers[idx];
}

inline const bmin::DynArray<TileInstance>* mapLayerPtr(
    const bmin::DynArray<bmin::DynArray<TileInstance>>& layers, int layer) {
  if (layer < 0 || static_cast<size_t>(layer) >= layers.size()) {
    return nullptr;
  }
  return &layers[static_cast<size_t>(layer)];
}

MapInstance createMapInstanceFromTemplate(const CarcerMapTemplate& mapTemplate);

// Flat cell index → tile (x, y); matches createMapInstanceFromTemplate math.
TileXY tileIndexToXY(int i, int width);

// First marker whose name matches (ceditor findMarkerOnMap semantics).
const MapMarkerPlacement* findMarkerOnTemplate(const CarcerMapTemplate& mapTemplate,
                                               const bmin::String& markerName);

CharacterInstance* findCharacterOnMap(MapInstance& map, const bmin::String& id);
const CharacterInstance* findCharacterOnMap(const MapInstance& map, const bmin::String& id);

CharacterInstance* findCharacterAt(MapInstance& map,
                                   int x,
                                   int y,
                                   const bmin::String& excludeId = bmin::String{});

} // namespace model
