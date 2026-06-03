#pragma once

#include <optional>
#include <string>
#include <vector>

namespace model {

struct CarcerMapTileTemplate;

struct MapTileItemEntry {
  std::string name;
  int quantity = 1;
};

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
  float angle = 0.0f;
  float intensity = 0.0f;
  int radius = 0;
};

struct TileOverrides {
  bool isWalkableOverride = false;
  bool isSeeThroughOverride = false;
  bool isContainerOverride = false;
  std::optional<TileLightSource> lightSourceOverride;
};

enum class MapType {
  TOWN,
  OUTDOOR,
};

std::string getStringFromMapType(MapType mapType);
MapType getMapTypeFromString(const std::string& mapTypeString);

struct CarcerMapTemplate {
  std::string name;
  std::string label;
  MapType type = MapType::TOWN;
  int width = 0;
  int height = 0;
  int spriteWidth = 28;
  int spriteHeight = 32;
  std::vector<CarcerMapTileTemplate> tiles;
};

struct CarcerMapTileTemplate {
  std::vector<std::string> characters;
  std::vector<MapTileItemEntry> items;
  std::vector<std::string> markers;
  std::optional<TileOverrides> tileOverrides;
  std::optional<TileLightSource> lightSource;
  std::optional<TileEventTrigger> eventTrigger;
  std::optional<TravelTrigger> travelTrigger;
  std::string tilesetName;
  int tileId = 0;
};

} // namespace model
