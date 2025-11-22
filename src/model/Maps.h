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

struct TravelTrigger {
  std::string destinationMapName;
  std::string destinationMarkerName;
  int destinationX = 0;
  int destinationY = 0;
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
  int spriteWidth = 28;
  int spriteHeight = 32;
  std::vector<CarcerMapTileTemplate> tiles;
};

struct CarcerMapTileTemplate {
  std::vector<std::string> characters;
  std::vector<std::string> items;
  std::vector<std::string> markers;
  std::optional<TileOverrides> tileOverrides;
  std::optional<TileLightSource> lightSource;
  std::optional<TileEventTrigger> eventTrigger;
  std::optional<TravelTrigger> travelTrigger;
  std::string tilesetName;
  int tileId;
};

} // namespace model