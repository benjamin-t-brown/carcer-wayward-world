#pragma once

#include "Tileset.h"
#include <string>
#include <vector>

namespace model {

struct TownTile;

struct MapOutdoorsTemplate {
  std::string templateName;
  int width;
  int height;
  std::vector<TownTile> tiles;
};

struct MapOutdoorsTile {
  int tileId;
  int x;
  int y;
  std::string tilesetName;
  TileOverrides tileOverrides;
  std::vector<std::string> wanderers;
  std::vector<std::string> events;
};

} // namespace model