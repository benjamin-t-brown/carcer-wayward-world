#include "db/Database.h"
#include "model/MapWalkability.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual,
                    const char* expected,
                    const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual.cStr()
               << LOG_ENDL;
    return false;
  }
  return true;
}

model::TileInstance makeTile(int x, int y, const char* tilesetName, int tileId) {
  auto tile = model::TileInstance{};
  tile.x = x;
  tile.y = y;
  tile.tilesetName = tilesetName;
  tile.tileId = tileId;
  return tile;
}

model::TileInstance makeEmptyTile(int x, int y) {
  auto tile = model::TileInstance{};
  tile.x = x;
  tile.y = y;
  return tile;
}

bmin::DynArray<model::TileInstance> makeEmptyLayer(int width, int height) {
  auto layerTiles = bmin::DynArray<model::TileInstance>{};
  layerTiles.reserve(static_cast<size_t>(width * height));
  for (auto y = 0; y < height; y++) {
    for (auto x = 0; x < width; x++) {
      layerTiles.pushBack(makeEmptyTile(x, y));
    }
  }
  return layerTiles;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestResolveTileToRender" << LOG_ENDL;
  auto ok = true;

  // Synthetic: layer 0 door, layer 1 wall at (1,1)
  {
    auto map = model::MapInstance{};
    map.width = 3;
    map.height = 3;
    map.tiles[0] = makeEmptyLayer(3, 3);
    map.tiles[1] = makeEmptyLayer(3, 3);
    map.tiles[0][4] = makeTile(1, 1, "terrain2", 4);
    map.tiles[1][4] = makeTile(1, 1, "terrain2", 0);

    map.tileLayerNumber = 0;
    const auto* tile0 = model::resolveTileToRender(map, 1, 1);
    ok = assertTrue(tile0 != nullptr, "layer0 present") && ok;
    if (tile0) {
      ok = assertEqualStr(tile0->tilesetName, "terrain2", "layer0 tileset") && ok;
      ok = assertEqual(tile0->tileId, 4, "layer0 tileId door") && ok;
    }

    map.tileLayerNumber = 1;
    const auto* tile1 = model::resolveTileToRender(map, 1, 1);
    ok = assertTrue(tile1 != nullptr, "layer1 present") && ok;
    if (tile1) {
      ok = assertEqualStr(tile1->tilesetName, "terrain2", "layer1 tileset") && ok;
      ok = assertEqual(tile1->tileId, 0, "layer1 tileId wall") && ok;
    }

    // Empty current layer falls through to highest below
    map.tiles[1][4] = makeEmptyTile(1, 1);
    map.tileLayerNumber = 1;
    const auto* fallthrough = model::resolveTileToRender(map, 1, 1);
    ok = assertTrue(fallthrough != nullptr, "fallthrough present") && ok;
    if (fallthrough) {
      ok = assertEqual(fallthrough->tileId, 4, "fallthrough door") && ok;
    }

    // Empty cell → null
    const auto* empty = model::resolveTileToRender(map, 0, 0);
    ok = assertTrue(empty == nullptr, "empty cell null") && ok;
  }

  // Real map repro: alinea_outsideAlinea1 index 250 (10,8)
  {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    database.load();

    const auto& mapTemplate = database.getMapTemplate("alinea_outsideAlinea1");
    auto map = model::createMapInstanceFromTemplate(mapTemplate);
    map.tileLayerNumber = 0;

    ok = assertEqual(map.width, 30, "map.width") && ok;
    const auto index = 250;
    const auto xy = model::tileIndexToXY(index, map.width);
    ok = assertEqual(xy.x, 10, "index250.x") && ok;
    ok = assertEqual(xy.y, 8, "index250.y") && ok;

    // Sanity: layer 0 door, layer 1 wall at that cell
    ok = assertTrue(map.tiles.contains(0), "has layer 0") && ok;
    ok = assertTrue(map.tiles.contains(1), "has layer 1") && ok;
    if (map.tiles.contains(0) && map.tiles.contains(1)) {
      const auto& layer0 = map.tiles[0][static_cast<size_t>(index)];
      const auto& layer1 = map.tiles[1][static_cast<size_t>(index)];
      ok = assertEqualStr(layer0.tilesetName, "terrain2", "raw layer0 tileset") && ok;
      ok = assertEqual(layer0.tileId, 4, "raw layer0 tileId") && ok;
      ok = assertEqualStr(layer1.tilesetName, "terrain2", "raw layer1 tileset") && ok;
      ok = assertEqual(layer1.tileId, 0, "raw layer1 tileId") && ok;
    }

    const auto* resolved = model::resolveTileToRender(map, xy.x, xy.y);
    ok = assertTrue(resolved != nullptr, "index250 resolved") && ok;
    if (resolved) {
      ok = assertEqualStr(resolved->tilesetName, "terrain2", "index250 tileset") && ok;
      ok = assertEqual(resolved->tileId, 4, "index250 tileId door") && ok;
      auto spriteName = resolved->tilesetName + "_" + bmin::toString(resolved->tileId);
      ok = assertEqualStr(spriteName, "terrain2_4", "index250 sprite") && ok;
    }
  }

  if (!ok) {
    LOG(ERROR) << "TestResolveTileToRender FAILED" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestResolveTileToRender PASSED" << LOG_ENDL;
  return 0;
}
