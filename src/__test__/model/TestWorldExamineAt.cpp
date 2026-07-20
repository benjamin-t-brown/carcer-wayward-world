#include "db/Database.h"
#include "model/MapWalkability.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/World.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/actions/world/WorldExamineAt.hpp"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertFalse(bool cond, const char* label) {
  if (cond) {
    LOG(ERROR) << label << " expected false" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual,
                    const bmin::String& expected,
                    const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected '" << expected << "' but got '" << actual << "'"
               << LOG_ENDL;
    return false;
  }
  return true;
}

model::TileMetadata makeMeta(int id, const char* description) {
  auto meta = model::TileMetadata{};
  meta.id = id;
  meta.description = description;
  meta.isWalkable = true;
  return meta;
}

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  tileset.tiles.pushBack(makeMeta(0, "grass"));
  database.addTilesetTemplate(tileset);
}

model::TileInstance makeTile(int x, int y, bool visible) {
  auto tile = model::TileInstance{};
  tile.x = x;
  tile.y = y;
  tile.tilesetName = "test_terrain";
  tile.tileId = 0;
  tile.isExplored = true;
  tile.isVisible = visible;
  return tile;
}

model::MapInstance makeMap(int width, int height, bool visible = true) {
  auto map = model::MapInstance{};
  map.width = width;
  map.height = height;
  map.tileLayerNumber = 0;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      layer.pushBack(makeTile(x, y, visible));
    }
  }
  model::mapLayerAt(map.tiles, 0) = std::move(layer);
  return map;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldExamineAt" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  addTestTileset(database);

  bool ok = true;

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
    state.world.actionMode = model::WorldActionMode::EXAMINE;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto& tile = state.world.currentMap.tiles[0][3 + 2 * 5];
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "look_event",
        .isLookTrigger = true,
    };

    state::actions::WorldExamineAt examineAt(3, 2);
    examineAt.execute(&state);

    ok = assertTrue(state.world.pendingSpecialEventId.has_value(),
                    "look trigger queues event") &&
         ok;
    if (state.world.pendingSpecialEventId) {
      ok = assertEqualStr(*state.world.pendingSpecialEventId, "look_event",
                          "look event id") &&
           ok;
    }
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "examine mode cleared after look trigger") &&
         ok;
    ok = assertFalse(state.world.actionAimTile.has_value(),
                     "aim cleared after look trigger") &&
         ok;
  }

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
    state.world.actionMode = model::WorldActionMode::EXAMINE;
    state.world.actionAimTile = model::TileXY{1, 1};

    state::actions::WorldExamineAt examineAt(1, 1);
    examineAt.execute(&state);

    ok = assertFalse(state.world.pendingSpecialEventId.has_value(),
                     "message path does not queue event") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "examine mode cleared after message") &&
         ok;
    ok = assertFalse(state.world.actionAimTile.has_value(),
                     "aim cleared after message") &&
         ok;
  }

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5, false);
    state.world.actionMode = model::WorldActionMode::EXAMINE;
    state.world.actionAimTile = model::TileXY{4, 4};

    auto& tile = state.world.currentMap.tiles[0][4 + 4 * 5];
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "hidden_look",
        .isLookTrigger = true,
    };

    ok = assertFalse(model::isTileCurrentlyVisible(state.world.currentMap, 4, 4),
                     "target not visible") &&
         ok;

    state::actions::WorldExamineAt examineAt(4, 4);
    examineAt.execute(&state);

    ok = assertFalse(state.world.pendingSpecialEventId.has_value(),
                     "non-visible does not queue look event") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::EXAMINE,
                    "examine mode kept when not visible") &&
         ok;
    ok = assertTrue(state.world.actionAimTile.has_value() &&
                        state.world.actionAimTile->x == 4 &&
                        state.world.actionAimTile->y == 4,
                    "aim kept when not visible") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestWorldExamineAt failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestWorldExamineAt completed successfully" << LOG_ENDL;
  return 0;
}
