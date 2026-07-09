#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/instances/ItemInstance.h"
#include "model/templates/Maps.h"
#include <optional>
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "lib/bmin/Map.h"

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
  bmin::Map<int, bmin::DynArray<TileInstance>> tiles;
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

struct World {
  bmin::String name;
  MapInstance currentMap;
};

} // namespace model
