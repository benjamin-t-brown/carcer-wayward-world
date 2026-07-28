#include "model/instances/MapInstance.h"
#include "model/templates/UtilityTypes.h"

namespace model {
namespace {

TileInstance* findTileAt(MapInstance& instance, int layer, int index) {
  const auto* layerTiles = mapLayerPtr(instance.tiles, layer);
  if (!layerTiles) {
    return nullptr;
  }
  if (index < 0 || index >= static_cast<int>(layerTiles->size())) {
    return nullptr;
  }
  return const_cast<TileInstance*>(&(*layerTiles)[static_cast<size_t>(index)]);
}

bool isValidPlacementIndex(const MapInstance& instance, int layer, int index) {
  const auto* layerTiles = mapLayerPtr(instance.tiles, layer);
  if (!layerTiles) {
    return false;
  }
  return index >= 0 && index < static_cast<int>(layerTiles->size());
}

} // namespace

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
  instance.turnMode = mapTemplate.type == MapType::OUTDOOR ? TurnMode::TURN_OUTDOOR
                                                           : TurnMode::TURN_TOWN;

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
    mapLayerAt(instance.tiles, static_cast<int>(layer)) = std::move(layerTiles);
  }

  for (const auto& ov : mapTemplate.tileOverrides) {
    auto* tile = findTileAt(instance, ov.l, ov.i);
    if (!tile) {
      continue;
    }
    tile->tileOverrides = ov.overrides;
  }

  for (const auto& et : mapTemplate.eventTriggers) {
    auto* tile = findTileAt(instance, et.l, et.i);
    if (!tile) {
      continue;
    }
    tile->eventTrigger = TileEventTrigger{
        .eventId = et.eventId,
        .requiresNonCombat = et.requiresNonCombat,
        .requiresLook = et.requiresLook,
    };
  }

  for (const auto& tt : mapTemplate.travelTriggers) {
    auto* tile = findTileAt(instance, tt.l, tt.i);
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
    };
  }

  for (const auto& ls : mapTemplate.lightSources) {
    auto* tile = findTileAt(instance, ls.l, ls.i);
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
    if (!isValidPlacementIndex(instance, placement.l, placement.i)) {
      continue;
    }
    auto tile = tileIndexToXY(placement.i, instance.width);
    auto character = CharacterInstance{};
    character.id = createRandomId();
    character.name = placement.name;
    character.templateName = placement.name;
    character.x = tile.x;
    character.y = tile.y;
    character.spawnX = tile.x;
    character.spawnY = tile.y;
    instance.characters.pushBack(std::move(character));
  }

  for (const auto& placement : mapTemplate.items) {
    if (!isValidPlacementIndex(instance, placement.l, placement.i)) {
      continue;
    }
    auto tile = tileIndexToXY(placement.i, instance.width);
    auto item = ItemInstance{};
    item.id = createRandomId();
    item.itemTemplateName = placement.name;
    item.quantity = placement.quantity;
    item.x = tile.x;
    item.y = tile.y;
    instance.items.pushBack(std::move(item));
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

CharacterInstance* findCharacterOnMap(MapInstance& map, const bmin::String& id) {
  for (size_t i = 0; i < map.characters.size(); i++) {
    if (map.characters[i].id == id) {
      return &map.characters[i];
    }
  }
  return nullptr;
}

const CharacterInstance* findCharacterOnMap(const MapInstance& map, const bmin::String& id) {
  return findCharacterOnMap(const_cast<MapInstance&>(map), id);
}

CharacterInstance* findCharacterAt(MapInstance& map,
                                   int x,
                                   int y,
                                   const bmin::String& excludeId) {
  for (size_t i = 0; i < map.characters.size(); i++) {
    auto& character = map.characters[i];
    if (character.x == x && character.y == y && character.id != excludeId) {
      return &character;
    }
  }
  return nullptr;
}

} // namespace model
