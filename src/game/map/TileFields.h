#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {
struct MapInstance;
struct TileInstance;
struct PersistentTileFieldRecord;
} // namespace model

namespace game {

enum class TileFieldType { BLOOD, FLAME, STATIC };

inline constexpr int TILE_FIELD_MOVES_PER_COMBAT_ROUND = 4;
inline constexpr int TILE_FIELD_BLOOD_MOVE_DURATION = 20;
inline constexpr int TILE_FIELD_FLAME_MOVE_DURATION = 30;
// STATIC and moveDuration 0: never expires via movement aging.

struct TileField {
  TileFieldType type = TileFieldType::BLOOD;
  // BLOOD only: random extra-sheet index 0–3 chosen when the field is added.
  int variant = 0;
  // Remaining player-movement ticks; decremented globally. 0 = permanent.
  int moveDuration = 0;
};

inline constexpr const char* TILE_FIELD_SPRITE_SHEET = "extra";

int tileFieldExtraSpriteIndex(const TileField& field);
bmin::String tileFieldSpriteName(const TileField& field);
int tileFieldDefaultMoveDuration(TileFieldType type);

void ageTileFields(bmin::DynArray<TileField>& fields, int steps);
void ageMapInstanceTileFields(model::MapInstance& map, int steps);
void agePersistentTileFieldRecords(bmin::DynArray<model::PersistentTileFieldRecord>& records,
                                   int steps);

void addTileField(model::TileInstance& tile, TileFieldType type);
void addTileFieldAt(model::MapInstance& map, int tileX, int tileY, TileFieldType type);

} // namespace game
