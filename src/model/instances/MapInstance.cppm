module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <cstdlib>

export module carcer.model.instances:MapInstance;
export import bmin.containers;
import bmin.string_interop;
export import carcer.game.map.TileFields;
export import :CharacterInstance;
export import :ItemInstance;
export import :TileInstance;
export import carcer.model.templates;
import carcer.model.templates;
import sdl2w;
#include "macros.h"

export {

// --- from model/instances/MapInstance.h ---
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
  const auto& tiles = mapInstanceTiles(map);
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

namespace game {

void ageMapInstanceTileFields(model::MapInstance& map, int steps);
void agePersistentTileFieldRecords(bmin::DynArray<model::PersistentTileFieldRecord>& records,
                                   int steps);
void addTileField(model::TileInstance& tile, TileFieldType type);
void addTileFieldAt(model::MapInstance& map, int tileX, int tileY, TileFieldType type);

} // namespace game

} // export

namespace game {

void ageMapInstanceTileFields(model::MapInstance& map, int steps) {
  if (steps <= 0) {
    return;
  }
  for (auto it = model::mapInstanceTiles(map).begin();
       it != model::mapInstanceTiles(map).end();
       ++it) {
    auto& layer = it->value;
    for (size_t ti = 0; ti < layer.size(); ti++) {
      ageTileFields(layer[ti].fields, steps);
    }
  }
}

void agePersistentTileFieldRecords(
    bmin::DynArray<model::PersistentTileFieldRecord>& records, int steps) {
  if (steps <= 0) {
    return;
  }
  for (size_t i = 0; i < records.size();) {
    ageTileFields(records[i].fields, steps);
    if (records[i].fields.empty()) {
      records.erase(i);
    } else {
      ++i;
    }
  }
}

void addTileField(model::TileInstance& tile, TileFieldType type) {
  TileField field;
  field.type = type;
  field.moveDuration = tileFieldDefaultMoveDuration(type);
  if (type == TileFieldType::BLOOD) {
    field.variant = std::rand() % 4;
    tile.fields.insert(tile.fields.begin(), field);
    return;
  }
  tile.fields.pushBack(field);
}

void addTileFieldAt(model::MapInstance& map, int tileX, int tileY, TileFieldType type) {
  auto* tile = model::mapInstanceGetTileAt(map, tileX, tileY, map.tileLayerNumber);
  if (tile == nullptr || tile->tilesetName.empty()) {
    return;
  }
  addTileField(*tile, type);
}

} // namespace game

namespace model {

int tileXYToIndex(int x, int y, int width) { return static_cast<int>(y * width + x); }

MapInstance createMapInstanceFromTemplate(const CarcerMapTemplate& mapTemplate) {
  auto instance = MapInstance{};
  instance.id = mapTemplate.name;
  instance.templateName = mapTemplate.name;
  instance.label = mapTemplate.label;
  instance.width = mapTemplate.width;
  instance.height = mapTemplate.height;
  instance.spriteWidth = mapTemplate.spriteWidth;
  instance.spriteHeight = mapTemplate.spriteHeight;
  instance.mapType = mapTemplate.type;

  auto cellCount = mapTemplate.width * mapTemplate.height;
  for (size_t layer = 0; layer < mapTemplate.tiles.size(); ++layer) {
    const auto& flat = mapTemplate.tiles[layer];
    if (flat.empty()) {
      continue;
    }
    auto layerTiles = bmin::DynArray<TileInstance>{};
    layerTiles.reserve(static_cast<size_t>(cellCount));

    for (auto i = 0; i < cellCount; i++) {
      auto tile = TileInstance{};
      tile.x = i % mapTemplate.width;
      tile.y = i / mapTemplate.width;

      auto pairIdx = i * 2;
      if (pairIdx + 1 < static_cast<int>(flat.size())) {
        auto tilesetIndex = flat[static_cast<size_t>(pairIdx)];
        auto tileId = flat[static_cast<size_t>(pairIdx + 1)];
        tile.tileId = tileId;
        if (tilesetIndex >= 0 &&
            tilesetIndex < static_cast<int>(mapTemplate.tilesets.size())) {
          tile.tilesetName = mapTemplate.tilesets[static_cast<size_t>(tilesetIndex)];
        }
      }
      layerTiles.pushBack(std::move(tile));
    }
    mapLayerAt(mapInstanceTiles(instance), static_cast<int>(layer)) =
        std::move(layerTiles);
  }

  for (const auto& ov : mapTemplate.tileOverrides) {
    auto [x, y] = tileIndexToXY(ov.i, instance.width);
    auto* tile = mapInstanceGetTileAt(instance, x, y, ov.l);
    if (!tile) {
      continue;
    }
    tile->tileOverrides = ov.overrides;
  }

  for (const auto& et : mapTemplate.eventTriggers) {
    auto [x, y] = tileIndexToXY(et.i, instance.width);
    auto* tile = mapInstanceGetTileAt(instance, x, y, et.l);
    if (!tile) {
      continue;
    }
    tile->eventTrigger = TileEventTrigger{
        .eventId = et.eventId,
        .requiresNonCombat = et.requiresNonCombat,
        .requiresLook = et.requiresLook,
        .overlayVisibility = et.overlayVisibility,
    };
  }

  for (const auto& tt : mapTemplate.travelTriggers) {
    auto [x, y] = tileIndexToXY(tt.i, instance.width);
    auto* tile = mapInstanceGetTileAt(instance, x, y, tt.l);
    if (!tile) {
      continue;
    }
    tile->travelTrigger = TravelTrigger{
        .destinationMapName = tt.destinationMapName,
        .destinationMarkerName = tt.destinationMarkerName,
        .destinationX = tt.destinationX,
        .destinationY = tt.destinationY,
        .destinationLayer = tt.destinationLayer,
        .requiresAction = tt.requiresAction,
        .overlayVisibility = tt.overlayVisibility,
    };
  }

  for (const auto& ls : mapTemplate.lightSources) {
    auto [x, y] = tileIndexToXY(ls.i, instance.width);
    auto* tile = mapInstanceGetTileAt(instance, x, y, ls.l);
    if (!tile) {
      continue;
    }
    tile->lightSource = TileLightSource{
        .angle = ls.angle,
        .intensity = ls.intensity,
        .radius = ls.radius,
    };
  }

  for (const auto& placement : mapTemplate.characters) {
    auto tile = tileIndexToXY(placement.i, instance.width);
    auto character = CharacterInstance{};
    character.id = createRandomId();
    character.name = placement.name;
    character.templateName = placement.name;
    character.x = tile.x;
    character.y = tile.y;
    character.spawnX = tile.x;
    character.spawnY = tile.y;
    instance.persistentState.characters.pushBack(std::move(character));
  }

  for (const auto& placement : mapTemplate.items) {
    auto tile = tileIndexToXY(placement.i, instance.width);
    auto item = ItemInstance{};
    item.id = createRandomId();
    item.itemTemplateName = placement.name;
    item.quantity = placement.quantity;
    item.x = tile.x;
    item.y = tile.y;
    instance.persistentState.items.pushBack(std::move(item));
  }

  return instance;
}

TileXY tileIndexToXY(int i, int width) {
  if (width <= 0) {
    return TileXY{};
  }
  return TileXY{.x = i % width, .y = i / width};
}

const MapMarkerPlacement* findMarkerOnTemplate(const CarcerMapTemplate& mapTemplate,
                                               const bmin::String& markerName) {
  for (const auto& marker : mapTemplate.markers) {
    if (marker.name == markerName) {
      return &marker;
    }
  }
  return nullptr;
}

CharacterInstance* mapInstanceFindCharacter(MapInstance& map, const bmin::String& id) {
  for (size_t i = 0; i < map.persistentState.characters.size(); i++) {
    if (map.persistentState.characters[i].id == id) {
      return &map.persistentState.characters[i];
    }
  }
  return nullptr;
}

const CharacterInstance* mapInstanceFindCharacter(const MapInstance& map,
                                                  const bmin::String& id) {
  return mapInstanceFindCharacter(const_cast<MapInstance&>(map), id);
}

} // namespace model
