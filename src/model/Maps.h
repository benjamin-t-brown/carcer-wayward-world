#pragma once

#include <optional>
#include <string>
#include <vector>

namespace model {

struct CarcerMapTileTemplate;

struct TileEventTrigger {
  std::string eventId;
  bool isNonCombatTrigger = true;
  bool isLookTrigger = false;
};

struct TileLightSource {
  float angle;
  float intensity;
  int radius;
};

struct TileOverrides {
  bool isWalkableOverride = false;
  bool isSeeThroughOverride = false;
  bool isContainerOverride = false;
  std::optional<TileLightSource> lightSourceOverride;
};

struct CarcerMapTemplate {
  std::string name;
  std::string label;
  int width;
  int height;
  std::vector<CarcerMapTileTemplate> tiles;
};

struct CarcerMapTileTemplate {
  std::vector<std::string> characters;
  std::vector<std::string> items;
  std::optional<TileOverrides> tileOverrides;
  std::optional<TileLightSource> lightSource;
  std::optional<TileEventTrigger> eventTrigger;
  std::string tilesetName;
  int tileId;
  int x;
  int y;
};

} // namespace model