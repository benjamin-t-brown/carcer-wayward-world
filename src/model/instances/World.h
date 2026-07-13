#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/instances/ItemInstance.h"
#include "model/templates/Maps.h"
#include <optional>
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

struct TileInstance {
  bmin::String id;
  bmin::String tilesetName;
  int tileId = 0;
  int x = 0;
  int y = 0;
  std::optional<TileOverrides> tileOverrides;
  std::optional<TileLightSource> lightSource;
  std::optional<TileEventTrigger> eventTrigger;
  std::optional<TravelTrigger> travelTrigger;
  bool isExplored = false;
  bool isVisible = false;
};

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

enum class CameraMode { Follow, Aiming, Dragging, Controlled };

enum class WorldActionMode { NONE, EXAMINE };

struct World {
  bmin::String name;
  MapInstance currentMap;
  int camX = 0; // map pixel space
  int camY = 0;
  CameraMode cameraMode = CameraMode::Follow;
  // empty = auto-resolve to current party member avatar when cameraMode is Follow
  bmin::String cameraFollowCharacterId;
  int viewW = 0; // MapView content size in map-pixel space (unscaled)
  int viewH = 0;
  WorldActionMode actionMode = WorldActionMode::NONE;
  std::optional<bmin::String> pendingSpecialEventId;
  std::optional<TravelTrigger> pendingTravel;
};

struct TileXY {
  int x = 0;
  int y = 0;
};

struct CameraPos {
  int camX = 0;
  int camY = 0;
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

// Center camera on target tile. Does not clamp — map may scroll past its edges
// so the target stays centered (empty space outside the map is allowed).
CameraPos computeCameraFollow(int targetTileX,
                              int targetTileY,
                              const MapInstance& map,
                              int viewW,
                              int viewH);

} // namespace model
