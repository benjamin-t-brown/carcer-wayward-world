module;
#include <cstddef>
#include <cstdint>
#include <utility>

module carcer.game.map.TileFields;
import bmin.containers;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace game {

int tileFieldExtraSpriteIndex(const TileField& field) {
  switch (field.type) {
  case TileFieldType::BLOOD:
    return field.variant;
  case TileFieldType::FLAME:
    return 4;
  case TileFieldType::STATIC:
    return 5;
  }
  return 0;
}

bmin::String tileFieldSpriteName(const TileField& field) {
  return bmin::String(TILE_FIELD_SPRITE_SHEET) + "_" +
         bmin::toString(tileFieldExtraSpriteIndex(field));
}

int tileFieldDefaultMoveDuration(TileFieldType type) {
  switch (type) {
  case TileFieldType::BLOOD:
    return TILE_FIELD_BLOOD_MOVE_DURATION;
  case TileFieldType::FLAME:
    return TILE_FIELD_FLAME_MOVE_DURATION;
  case TileFieldType::STATIC:
    return 0;
  }
  return 0;
}

void ageTileFields(bmin::DynArray<TileField>& fields, int steps) {
  if (steps <= 0) {
    return;
  }
  for (size_t i = 0; i < fields.size();) {
    auto& field = fields[i];
    if (field.moveDuration > 0) {
      field.moveDuration -= steps;
      if (field.moveDuration <= 0) {
        fields.erase(i);
        continue;
      }
    }
    ++i;
  }
}

} // namespace game
