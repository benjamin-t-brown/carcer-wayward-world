#include "db/Database.h"
#include "model/TileTriggers.h"
#include "model/instances/World.h"
#include "model/templates/Items.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/actions/world/WorldMovePlayer.hpp"
#include "bmin/String.h"

namespace {

bool assertEqualStr(const bmin::String& actual, const bmin::String& expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected '" << expected << "' but got '" << actual << "'" << LOG_ENDL;
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

void addTestItem(db::Database& database) {
  auto item = model::ItemTemplate{};
  item.name = "TestBeer";
  item.label = "Test Beer";
  database.addItemTemplate(item);
}

model::TileInstance makeTile(int x, int y, int tileId) {
  auto tile = model::TileInstance{};
  tile.x = x;
  tile.y = y;
  tile.tilesetName = "test_terrain";
  tile.tileId = tileId;
  return tile;
}

model::MapInstance makeMap(int width, int height) {
  auto map = model::MapInstance{};
  map.width = width;
  map.height = height;
  map.tileLayerNumber = 0;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      layer.pushBack(makeTile(x, y, 0));
    }
  }
  model::mapLayerAt(map.tiles, 0) = std::move(layer);
  return map;
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestTileTriggers" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  addTestTileset(database);
  addTestItem(database);

  bool ok = true;

  {
    auto world = model::World{};
    auto map = makeMap(2, 2);
    auto& tile = map.tiles[0][0];
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "step_event",
        .isLookTrigger = false,
    };
    tile.travelTrigger = model::TravelTrigger{
        .destinationMapName = "other",
        .destinationMarkerName = "door",
    };
    world.currentMap = map;

    model::queueStepTriggersAt(world, map, 0, 0);
    ok = assertTrue(world.pendingSpecialEventId.has_value(), "event takes precedence") && ok;
    ok = assertEqualStr(*world.pendingSpecialEventId, "step_event", "event id") && ok;
    ok = assertTrue(!world.pendingTravel.has_value(), "travel ignored when event present") && ok;
  }

  {
    auto world = model::World{};
    auto map = makeMap(2, 2);
    map.tiles[0][0].travelTrigger = model::TravelTrigger{
        .destinationMapName = "dest_map",
        .destinationX = 3,
        .destinationY = 4,
    };
    world.currentMap = map;

    model::queueStepTriggersAt(world, map, 0, 0);
    ok = assertTrue(!world.pendingSpecialEventId.has_value(), "no event pending") && ok;
    ok = assertTrue(world.pendingTravel.has_value(), "travel pending") && ok;
    ok = assertEqualStr(world.pendingTravel->destinationMapName, "dest_map", "travel map") && ok;
  }

  {
    auto map = makeMap(2, 2);
    map.items.pushBack(model::ItemInstance{
        .itemTemplateName = "TestBeer",
        .x = 1,
        .y = 0,
    });
    const auto message = model::formatExamineMessage(map, 1, 0, database);
    ok = assertEqualStr(message, "Examine:\ngrass\nTest Beer", "examine message with item") && ok;
  }

  {
    state::State state{};
    state.world.currentMap = makeMap(3, 3);
    auto& map = state.world.currentMap;

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    auto character = model::CharacterInstance{};
    character.id = "player1";
    character.x = 1;
    character.y = 1;
    map.characters.pushBack(character);

    auto& destTile = map.tiles[0][static_cast<size_t>(1 * map.width + 2)];
    destTile.eventTrigger =
        model::TileEventTrigger{.eventId = "on_step", .isLookTrigger = false};

    state::actions::WorldMovePlayer moveEast(1, 0);
    moveEast.execute(&state);

    ok = assertTrue(state.world.pendingSpecialEventId.has_value(), "move queues step event") && ok;
    ok = assertEqualStr(*state.world.pendingSpecialEventId, "on_step", "step event id after move") &&
         ok;
    ok = assertTrue(map.characters[0].x == 2 && map.characters[0].y == 1, "avatar moved east") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestTileTriggers failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestTileTriggers completed successfully" << LOG_ENDL;
  return 0;
}
