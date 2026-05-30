#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/instances/ItemInstance.h"
#include "model/templates/Maps.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace model {

struct TileInstance {
  std::string id;
  std::string tilesetName;
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
  std::string id;
  std::string label;
  std::string templateName;
  std::unordered_map<int, std::vector<TileInstance>> tiles;
  std::vector<CharacterInstance> characters;
  std::vector<ItemInstance> items;
  int width = 0;
  int height = 0;
  int spriteWidth = 0;
  int spriteHeight = 0;
  int tileLayerNumber = 0;
  MapType mapType = MapType::TOWN;
  TurnMode turnMode = TurnMode::TURN_TOWN;
};

struct World {
  std::string name;
  MapInstance currentMap;
};

} // namespace model
