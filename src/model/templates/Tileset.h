#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"

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
  bmin::String description = "";
  TileStepSound stepSound = TILE_STEP_SOUND_FLOOR;
  bool isWalkable = true;
  bool isSeeThrough = true;
  bool isDoor = false;
  bool isContainer = false;
};

struct TilesetTemplate {
  bmin::String name;
  bmin::String spriteBase;
  int tileWidth;
  int tileHeight;
  bmin::DynArray<TileMetadata> tiles;
};

} // namespace model
