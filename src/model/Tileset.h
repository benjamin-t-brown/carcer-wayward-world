#pragma once

#include <string>
#include <vector>

namespace model {

struct TileMetadata {
  int id;
  std::string description = "";
  bool isWalkable = true;
  bool isSeeThrough = true;
  bool isDoor = false;
};

struct Tileset {
  std::string name;
  std::string spriteBase;
  int tileWidth;
  int tileHeight;
  std::vector<TileMetadata> tiles;
};

struct TileOverrides {
  bool isWalkableOverride = false;
  bool isSeeThroughOverride = false;
};

} // namespace model