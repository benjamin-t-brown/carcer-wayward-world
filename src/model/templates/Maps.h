#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace model {

struct CarcerMapTileTemplate;

struct MapTileRef {
  int l = 0;
  int i = 0;
};

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

struct MapCharacterPlacement : MapTileRef {
  std::string name;
};

struct MapItemPlacement : MapTileRef {
  std::string name;
  int quantity = 1;
};

struct MapMarkerPlacement : MapTileRef {
  std::string name;
};

struct MapEventTriggerPlacement : MapTileRef, TileEventTrigger {};

struct MapTravelTriggerPlacement : MapTileRef, TravelTrigger {};

struct MapTileOverridePlacement : MapTileRef {
  TileOverrides overrides;
};

struct MapLightSourcePlacement : MapTileRef, TileLightSource {};

struct CarcerMapTemplate {
  std::string name;
  std::string label;
  MapType type = MapType::TOWN;
  int width = 0;
  int height = 0;
  int spriteWidth = 28;
  int spriteHeight = 32;
  std::vector<std::string> tilesets;
  std::vector<int> layers;
  std::unordered_map<int, std::vector<int>> tiles;
  std::vector<MapCharacterPlacement> characters;
  std::vector<MapItemPlacement> items;
  std::vector<MapMarkerPlacement> markers;
  std::vector<MapEventTriggerPlacement> eventTriggers;
  std::vector<MapTravelTriggerPlacement> travelTriggers;
  std::vector<MapTileOverridePlacement> tileOverrides;
  std::vector<MapLightSourcePlacement> lightSources;
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
