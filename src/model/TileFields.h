#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

struct MapInstance;
struct TileInstance;

enum class TileFieldType { BLOOD, FLAME, STATIC };

struct TileField {
  TileFieldType type = TileFieldType::BLOOD;
  // BLOOD only: random extra-sheet index 0–3 chosen when the field is added.
  int variant = 0;
};

inline constexpr const char* TILE_FIELD_SPRITE_SHEET = "extra";

int tileFieldExtraSpriteIndex(const TileField& field);
bmin::String tileFieldSpriteName(const TileField& field);

void addTileField(TileInstance& tile, TileFieldType type);
void addTileFieldAt(MapInstance& map, int tileX, int tileY, TileFieldType type);

} // namespace model
