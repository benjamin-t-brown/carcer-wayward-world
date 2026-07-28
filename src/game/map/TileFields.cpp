#include "game/map/TileFields.h"
#include "game/map/MapWalkability.h"
#include "model/instances/World.h"
#include <cstdlib>

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

void ageMapInstanceTileFields(model::MapInstance& map, int steps) {
  if (steps <= 0) {
    return;
  }
  for (size_t layerIdx = 0; layerIdx < map.tiles.size(); layerIdx++) {
    auto& layer = map.tiles[layerIdx];
    for (size_t ti = 0; ti < layer.size(); ti++) {
      ageTileFields(layer[ti].fields, steps);
    }
  }
}

void agePersistentTileFieldRecords(
    bmin::DynArray<model::PersistentTileFieldRecord>& records, int steps) {
  if (steps <= 0) {
    return;
  }
  for (size_t i = 0; i < records.size();) {
    ageTileFields(records[i].fields, steps);
    if (records[i].fields.empty()) {
      records.erase(i);
    } else {
      ++i;
    }
  }
}

void addTileField(model::TileInstance& tile, TileFieldType type) {
  TileField field;
  field.type = type;
  field.moveDuration = tileFieldDefaultMoveDuration(type);
  if (type == TileFieldType::BLOOD) {
    field.variant = std::rand() % 4;
    tile.fields.insert(tile.fields.begin(), field);
    return;
  }
  tile.fields.pushBack(field);
}

void addTileFieldAt(model::MapInstance& map, int tileX, int tileY, TileFieldType type) {
  auto* tile = tileAtCurrentLayer(map, tileX, tileY);
  if (tile == nullptr) {
    return;
  }
  addTileField(*tile, type);
}

} // namespace game
