#include "model/MapWalkability.h"
#include "model/TileFields.h"
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
  model::mapLayerAt(map.tiles, 0) = std::move(layer);
  return map;
}

} // namespace

int main() {
  LOG(INFO) << "Starting TestTileFields" << LOG_ENDL;
  auto ok = true;

  {
    model::TileField flameField{.type = model::TileFieldType::FLAME};
    ok = assertEqual(model::tileFieldExtraSpriteIndex(flameField), 4, "flame sprite index") && ok;
    ok = assertEqualStr(model::tileFieldSpriteName(flameField), "extra_4", "flame sprite name") &&
         ok;

    model::TileField staticField{.type = model::TileFieldType::STATIC};
    ok = assertEqual(model::tileFieldExtraSpriteIndex(staticField), 5, "static sprite index") &&
         ok;
  }

  {
    auto map = makeMap();
    model::addTileFieldAt(map, 1, 1, model::TileFieldType::FLAME);
    model::addTileFieldAt(map, 1, 1, model::TileFieldType::BLOOD);
    auto* tile = model::tileAtCurrentLayer(map, 1, 1);
    ok = assertTrue(tile != nullptr, "surface tile exists") && ok;
    if (tile) {
      ok = assertEqual(static_cast<int>(tile->fields.size()), 2, "field count") && ok;
      ok = assertTrue(tile->fields[0].type == model::TileFieldType::BLOOD, "blood at bottom") &&
           ok;
      ok = assertTrue(tile->fields[1].type == model::TileFieldType::FLAME, "flame on top") && ok;
      ok = assertTrue(tile->fields[0].variant >= 0 && tile->fields[0].variant < 4,
                      "blood variant in range") &&
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
