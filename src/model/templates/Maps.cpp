#include "model/templates/Maps.h"

#include <stdexcept>

namespace model {

bmin::String getStringFromMapType(MapType mapType) {
  switch (mapType) {
  case MapType::TOWN:
    return "TOWN";
  case MapType::OUTDOOR:
    return "OUTDOOR";
  default:
    throw std::runtime_error("Invalid map type");
  }
}

MapType getMapTypeFromString(const bmin::String& mapTypeString) {
  if (mapTypeString == "TOWN") {
    return MapType::TOWN;
  } else if (mapTypeString == "OUTDOOR") {
    return MapType::OUTDOOR;
  }
  throw std::runtime_error(("Invalid map type: " + mapTypeString).cStr());
}

bmin::String getStringFromTileOverlayVisibility(TileOverlayVisibility visibility) {
  switch (visibility) {
  case TileOverlayVisibility::HIDDEN:
    return "HIDDEN";
  case TileOverlayVisibility::SHOW_EVENT_ON_TILE:
    return "SHOW_EVENT_ON_TILE";
  case TileOverlayVisibility::SHOW_TRAVEL_UP:
    return "SHOW_TRAVEL_UP";
  case TileOverlayVisibility::SHOW_TRAVEL_DOWN:
    return "SHOW_TRAVEL_DOWN";
  default:
    return "HIDDEN";
  }
}

TileOverlayVisibility getTileOverlayVisibilityFromString(const bmin::String& value) {
  if (value == "SHOW_EVENT_ON_TILE") {
    return TileOverlayVisibility::SHOW_EVENT_ON_TILE;
  }
  if (value == "SHOW_TRAVEL_UP") {
    return TileOverlayVisibility::SHOW_TRAVEL_UP;
  }
  if (value == "SHOW_TRAVEL_DOWN") {
    return TileOverlayVisibility::SHOW_TRAVEL_DOWN;
  }
  return TileOverlayVisibility::HIDDEN;
}

bmin::String tileOverlayVisibilitySpriteName(TileOverlayVisibility visibility) {
  switch (visibility) {
  case TileOverlayVisibility::SHOW_EVENT_ON_TILE:
    return "extra_4";
  case TileOverlayVisibility::SHOW_TRAVEL_UP:
    return "extra_5";
  case TileOverlayVisibility::SHOW_TRAVEL_DOWN:
    return "extra_6";
  case TileOverlayVisibility::HIDDEN:
  default:
    return bmin::String{};
  }
}

} // namespace model
