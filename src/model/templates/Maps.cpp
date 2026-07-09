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

} // namespace model
