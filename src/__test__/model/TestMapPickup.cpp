#include "db/Database.h"
#include "game/map/MapPathfinding.h"
#include "game/map/MapPickup.h"
#include "game/map/MapWalkability.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/ItemInstance.h"
#include "model/instances/MapInstance.h"
#include "model/templates/MapGrids.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "bmin/String.h"

#define TEST_NAME "TestMapPickup"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
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

void addWalkableTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  auto meta = model::TileMetadata{};
  meta.id = 0;
  meta.isWalkable = true;
  meta.isSeeThrough = true;
  tileset.tiles.pushBack(meta);
  auto wall = model::TileMetadata{};
  wall.id = 1;
  wall.isWalkable = false;
  wall.isSeeThrough = false;
  tileset.tiles.pushBack(wall);
  auto crate = model::TileMetadata{};
  crate.id = 2;
  crate.isWalkable = false;
  crate.isSeeThrough = true;
  crate.isContainer = true;
  tileset.tiles.pushBack(crate);
  database.addTilesetTemplate(tileset);
}

model::MapInstance makeMap(int width, int height) {
  auto map = model::MapInstance{};
  map.id = "pickup_test_map";
  map.templateName = "pickup_test_map";
  map.width = width;
  map.height = height;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  map.tileLayerNumber = 0;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (auto y = 0; y < height; y++) {
    for (auto x = 0; x < width; x++) {
      auto tile = model::TileInstance{};
      tile.x = x;
      tile.y = y;
      tile.tilesetName = "test_terrain";
      tile.tileId = 0;
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  return map;
}

void setWall(model::MapInstance& map, int x, int y) {
  auto* tile = game::tileAtCurrentLayer(map, x, y);
  if (tile) {
    tile->tileId = 1;
  }
}

void setContainer(model::MapInstance& map, int x, int y) {
  auto* tile = game::tileAtCurrentLayer(map, x, y);
  if (tile) {
    tile->tileId = 2;
  }
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;
  bool ok = true;

  db::Database database;
  addWalkableTileset(database);
  state::DatabaseInterface::setDatabase(&database);

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  auto& state = stateManager.getState();

  auto map = makeMap(8, 3);
  // Wall barrier: only path east is blocked at x=3, so (4,*) is unreachable in 4 steps
  // when forced around... seal a dead-end room for the blocked item.
  setWall(map, 3, 0);
  setWall(map, 3, 1);
  setWall(map, 3, 2);
  // Container next to hero; item on it must not appear in Get list.
  setContainer(map, 1, 1);
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "pickup_test_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 8;
  grid.mapHeight = 3;
  grid.cells = {{"pickup_test_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "pickup_test_grid";

  state.world.activeMap.characters.pushBack(model::CharacterInstance{
      .id = "hero",
      .x = 0,
      .y = 1,
  });
  state.world.activeMap.items.pushBack(model::ItemInstance{
      .id = "near",
      .itemTemplateName = "PotionHealing",
      .quantity = 1,
      .x = 2,
      .y = 1,
  });
  state.world.activeMap.items.pushBack(model::ItemInstance{
      .id = "in_crate",
      .itemTemplateName = "PotionHealing",
      .quantity = 1,
      .x = 1,
      .y = 1,
  });
  state.world.activeMap.items.pushBack(model::ItemInstance{
      .id = "far",
      .itemTemplateName = "PotionHealing",
      .quantity = 1,
      .x = 7,
      .y = 1,
  });
  state.world.activeMap.items.pushBack(model::ItemInstance{
      .id = "blocked",
      .itemTemplateName = "PotionHealing",
      .quantity = 1,
      .x = 5,
      .y = 1,
  });

  const auto& hero = state.world.activeMap.characters[0];
  ok = assertTrue(
      game::isActiveMapTileContainer(state.world.activeMap, 1, 1, database),
      "crate is container") &&
       ok;

  const auto reachable = game::collectReachableTiles(
      state.world.activeMap, hero, game::PICKUP_PATH_RANGE, database);
  ok = assertTrue(game::isTileInReachableSet(reachable, 2, 1), "near tile reachable") &&
       ok;
  ok = assertTrue(!game::isTileInReachableSet(reachable, 5, 1),
                  "blocked tile unreachable") &&
       ok;
  ok = assertTrue(!game::isTileInReachableSet(reachable, 7, 1), "far tile unreachable") &&
       ok;

  const auto nearby = game::collectItemsWithinPickupRange(
      state.world.activeMap, hero, game::PICKUP_PATH_RANGE, database);

  ok = assertEqual(static_cast<int>(nearby.size()), 1, "nearby count") && ok;
  if (!nearby.empty()) {
    ok = assertTrue(nearby[0].id == "near", "nearby item id") && ok;
  }

  const auto containerItems =
      game::collectItemsAtActiveMapTile(state.world.activeMap, 1, 1);
  ok = assertEqual(static_cast<int>(containerItems.size()), 1, "container count") && ok;
  if (!containerItems.empty()) {
    ok = assertTrue(containerItems[0].id == "in_crate", "container item id") && ok;
  }

  state::StateManagerInterface::setStateManager(nullptr);
  state::DatabaseInterface::setDatabase(nullptr);

  if (!ok) {
    LOG(ERROR) << TEST_NAME << " assertions failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "Finished " << TEST_NAME << LOG_ENDL;
  return 0;
}
