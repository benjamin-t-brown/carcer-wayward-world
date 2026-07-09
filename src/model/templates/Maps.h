#pragma once

#include <optional>
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "lib/bmin/Map.h"

namespace model {

struct CarcerMapTileTemplate;

struct MapTileRef {
  int l = 0;
  int i = 0;
};

struct MapTileItemEntry {
  bmin::String name;
  int quantity = 1;
};

struct TileEventTrigger {
  bmin::String eventId;
  bool isNonCombatTrigger = true;
  bool isLookTrigger = false;
};

struct TravelTrigger {
  bmin::String destinationMapName;
  bmin::String destinationMarkerName;
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

bmin::String getStringFromMapType(MapType mapType);
MapType getMapTypeFromString(const bmin::String& mapTypeString);

struct MapCharacterPlacement : MapTileRef {
  bmin::String name;
};

struct MapItemPlacement : MapTileRef {
  bmin::String name;
  int quantity = 1;
};

struct MapMarkerPlacement : MapTileRef {
  bmin::String name;
};

struct MapEventTriggerPlacement : MapTileRef, TileEventTrigger {};

struct MapTravelTriggerPlacement : MapTileRef, TravelTrigger {};

struct MapTileOverridePlacement : MapTileRef {
  TileOverrides overrides;
};

struct MapLightSourcePlacement : MapTileRef, TileLightSource {};

struct CarcerMapTemplate {
  bmin::String name;
  bmin::String label;
  MapType type = MapType::TOWN;
  int width = 0;
  int height = 0;
  int spriteWidth = 28;
  int spriteHeight = 32;
  bmin::DynArray<bmin::String> tilesets;
  bmin::DynArray<int> layers;
  bmin::Map<int, bmin::DynArray<int>> tiles;
  bmin::DynArray<MapCharacterPlacement> characters;
  bmin::DynArray<MapItemPlacement> items;
  bmin::DynArray<MapMarkerPlacement> markers;
  bmin::DynArray<MapEventTriggerPlacement> eventTriggers;
  bmin::DynArray<MapTravelTriggerPlacement> travelTriggers;
  bmin::DynArray<MapTileOverridePlacement> tileOverrides;
  bmin::DynArray<MapLightSourcePlacement> lightSources;
};

struct CarcerMapTileTemplate {
  bmin::DynArray<bmin::String> characters;
  bmin::DynArray<MapTileItemEntry> items;
  bmin::DynArray<bmin::String> markers;
  std::optional<TileOverrides> tileOverrides;
  std::optional<TileLightSource> lightSource;
  std::optional<TileEventTrigger> eventTrigger;
  std::optional<TravelTrigger> travelTrigger;
  bmin::String tilesetName;
  int tileId = 0;
};

} // namespace model
