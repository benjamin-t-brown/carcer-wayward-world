#pragma once

#include <string>
#include <vector>

namespace model {

struct TileMetadata;

enum TileStepSound {
  TILE_STEP_SOUND_FLOOR,
  TILE_STEP_SOUND_GRASS,
  TILE_STEP_SOUND_DIRT,
  TILE_STEP_SOUND_GRAVEL,
};

struct TileMetadata {
  int id;
  std::string description = "";
  TileStepSound stepSound = TILE_STEP_SOUND_FLOOR;
  bool isWalkable = true;
  bool isSeeThrough = true;
  bool isDoor = false;
  bool isContainer = false;
};

struct TilesetTemplate {
  std::string name;
  std::string spriteBase;
  int tileWidth;
  int tileHeight;
  std::vector<TileMetadata> tiles;
};

} // namespace model