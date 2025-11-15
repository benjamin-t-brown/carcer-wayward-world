#pragma once 

#include "Tileset.h"
#include <vector>
#include <string>

namespace model{

struct TownTile;

struct MapTownTemplate {
  std::string templateName;
  int width;
  int height;
  std::vector<TownTile> tiles;
};

struct TownTile {
  int tileId;
  int x;
  int y;
  std::string tilesetName;
  TileOverrides tileOverrides;
  std::vector<std::string> characters;
  std::vector<std::string> items;
  std::vector<std::string> events;
};

}