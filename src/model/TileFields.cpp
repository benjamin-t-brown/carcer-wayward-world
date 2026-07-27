#include "model/TileFields.h"
#include "model/MapWalkability.h"
#include "model/instances/World.h"
#include <cstdlib>

namespace model {

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

void addTileField(TileInstance& tile, TileFieldType type) {
  TileField field;
  field.type = type;
  if (type == TileFieldType::BLOOD) {
    field.variant = std::rand() % 4;
    tile.fields.insert(tile.fields.begin(), field);
    return;
  }
  tile.fields.pushBack(field);
}

void addTileFieldAt(MapInstance& map, int tileX, int tileY, TileFieldType type) {
  auto* tile = tileAtCurrentLayer(map, tileX, tileY);
  if (tile == nullptr) {
    return;
  }
  addTileField(*tile, type);
}

} // namespace model
