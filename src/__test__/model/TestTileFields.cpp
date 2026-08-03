#include "game/map/MapWalkability.h"
#include "game/map/TileFields.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual, const bmin::String& expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

model::MapInstance makeMap() {
  auto map = model::MapInstance{};
  map.width = 3;
  map.height = 3;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  map.tileLayerNumber = 0;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (auto y = 0; y < 3; y++) {
    for (auto x = 0; x < 3; x++) {
      auto tile = model::TileInstance{};
      tile.x = x;
      tile.y = y;
      tile.tilesetName = "terrain0";
      tile.tileId = 1;
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  return map;
}

} // namespace

int main() {
  LOG(INFO) << "Starting TestTileFields" << LOG_ENDL;
  auto ok = true;

  {
    game::TileField flameField{.type = game::TileFieldType::FLAME};
    ok = assertEqual(game::tileFieldExtraSpriteIndex(flameField), 4, "flame sprite index") && ok;
    ok = assertEqualStr(game::tileFieldSpriteName(flameField), "extra_4", "flame sprite name") &&
         ok;

    game::TileField staticField{.type = game::TileFieldType::STATIC};
    ok = assertEqual(game::tileFieldExtraSpriteIndex(staticField), 5, "static sprite index") &&
         ok;
  }

  {
    auto map = makeMap();
    game::addTileFieldAt(map, 1, 1, game::TileFieldType::FLAME);
    game::addTileFieldAt(map, 1, 1, game::TileFieldType::BLOOD);
    auto* tile = game::tileAtCurrentLayer(map, 1, 1);
    ok = assertTrue(tile != nullptr, "surface tile exists") && ok;
    if (tile) {
      ok = assertEqual(static_cast<int>(tile->fields.size()), 2, "field count") && ok;
      ok = assertTrue(tile->fields[0].type == game::TileFieldType::BLOOD, "blood at bottom") &&
           ok;
      ok = assertTrue(tile->fields[1].type == game::TileFieldType::FLAME, "flame on top") && ok;
      ok = assertTrue(tile->fields[0].variant >= 0 && tile->fields[0].variant < 4,
                      "blood variant in range") &&
           ok;
      ok = assertEqual(tile->fields[0].moveDuration, game::TILE_FIELD_BLOOD_MOVE_DURATION,
                       "blood move duration") &&
           ok;
      ok = assertEqual(tile->fields[1].moveDuration, game::TILE_FIELD_FLAME_MOVE_DURATION,
                       "flame move duration") &&
           ok;
    }
  }

  if (ok) {
    LOG(INFO) << "TestTileFields PASSED" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << "TestTileFields FAILED" << LOG_ENDL;
  return 1;
}
