module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <utility>

module carcer.model;

import bmin.string_interop;

#include "macros.h"

// --- MapInstance.cppm ---

namespace model {

TileLayerMap& mapInstanceTiles(MapInstance& map) { return map.persistentState.tiles; }

const TileLayerMap& mapInstanceTiles(const MapInstance& map) {
  return map.persistentState.tiles;
}

bool mapHasLayer(const TileLayerMap& layers, int layer) { return layers.contains(layer); }

bool mapInstanceHasLayer(const TileLayerMap& layers, int layer) {
  return mapHasLayer(layers, layer);
}

bmin::DynArray<TileInstance>& mapLayerAt(TileLayerMap& layers, int layer) {
  if (!layers.contains(layer)) {
    layers[layer] = bmin::DynArray<TileInstance>{};
  }
  return layers[layer];
}

const bmin::DynArray<TileInstance>* mapLayerPtr(const TileLayerMap& layers, int layer) {
  if (!layers.contains(layer)) {
    return nullptr;
  }
  auto& mutableLayers = const_cast<TileLayerMap&>(layers);
  return &mutableLayers[layer];
}

bmin::DynArray<TileInstance>* mapLayerPtr(TileLayerMap& layers, int layer) {
  if (!layers.contains(layer)) {
    return nullptr;
  }
  return &layers[layer];
}

TileInstance* mapInstanceGetTileAt(MapInstance& map, int x, int y, int layer) {
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

const TileInstance* mapInstanceGetTileAt(const MapInstance& map, int x, int y, int layer) {
  return mapInstanceGetTileAt(const_cast<MapInstance&>(map), x, y, layer);
}

TileXY mapInstanceGetMinMaxLayer(const MapInstance& map) {
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

} // namespace model

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
