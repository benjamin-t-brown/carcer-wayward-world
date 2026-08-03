#include "db/Database.h"
#include "game/map/TileTriggers.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/MapGrids.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/actions/world/WorldMovePlayer.hpp"
#include "bmin/String.h"

namespace {

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

void addTestCharacter(db::Database& database) {
  auto character = model::CharacterTemplate{};
  character.name = "TestNpc";
  character.label = "Friendly NPC";
  database.addCharacterTemplate(character);
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
  map.id = "test_map";
  map.templateName = "test_map";
  map.width = width;
  map.height = height;
  map.tileLayerNumber = 0;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      layer.pushBack(makeTile(x, y, 0));
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  return map;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestTileTriggers" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  addTestTileset(database);
  addTestItem(database);
  addTestCharacter(database);

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  bool ok = true;

  {
    auto triggers = state::Triggers{};
    auto map = makeMap(2, 2);
    auto& tile = model::mapLayerAt(model::mapInstanceTiles(map), 0)[0];
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "step_event",
        .requiresLook = false,
    };
    tile.travelTrigger = model::TravelTrigger{
        .destinationMapName = "other",
        .destinationMarkerName = "door",
    };

    game::queueStepTriggersAt(triggers, map, 0, 0);
    ok = assertTrue(triggers.pendingSpecialEventId.has_value(), "event takes precedence") &&
         ok;
    ok = assertEqualStr(*triggers.pendingSpecialEventId, "step_event", "event id") && ok;
    ok = assertTrue(!triggers.pendingTravel.has_value(),
                    "travel ignored when event present") &&
         ok;
  }

  {
    auto triggers = state::Triggers{};
    auto map = makeMap(2, 2);
    model::mapLayerAt(model::mapInstanceTiles(map), 0)[0].travelTrigger =
        model::TravelTrigger{
            .destinationMapName = "dest_map",
            .destinationX = 3,
            .destinationY = 4,
        };

    game::queueStepTriggersAt(triggers, map, 0, 0);
    ok = assertTrue(!triggers.pendingSpecialEventId.has_value(), "no event pending") && ok;
    ok = assertTrue(triggers.pendingTravel.has_value(), "travel pending") && ok;
    ok = assertEqualStr(triggers.pendingTravel->destinationMapName, "dest_map",
                        "travel map") &&
         ok;
  }

  {
    auto triggers = state::Triggers{};
    auto map = makeMap(2, 2);
    model::mapLayerAt(model::mapInstanceTiles(map), 0)[0].travelTrigger =
        model::TravelTrigger{
            .destinationMapName = "action_dest",
            .requiresAction = true,
        };

    game::queueStepTriggersAt(triggers, map, 0, 0);
    ok = assertTrue(!triggers.pendingTravel.has_value(),
                    "action travel not queued on step") &&
         ok;

    game::queueActionTravelAtStanding(triggers, map, 0, 0);
    ok = assertTrue(triggers.pendingTravel.has_value(),
                    "action travel queued on interact") &&
         ok;
    ok = assertEqualStr(triggers.pendingTravel->destinationMapName, "action_dest",
                        "action travel map") &&
         ok;
  }

  {
    auto map = makeMap(2, 2);
    model::ActiveMap activeMap;
    activeMap.items.pushBack(model::ItemInstance{
        .itemTemplateName = "TestBeer",
        .x = 1,
        .y = 0,
    });
    const auto message = game::formatExamineMessage(map, activeMap, 1, 0, 1, 0, database);
    ok = assertEqualStr(message, "Examine:\ngrass\nTest Beer",
                        "examine message with item") &&
         ok;
  }

  {
    auto map = makeMap(2, 2);
    model::ActiveMap activeMap;
    activeMap.characters.pushBack(model::CharacterInstance{
        .id = "npc1",
        .name = "ignored instance name",
        .templateName = "TestNpc",
        .x = 1,
        .y = 0,
    });
    activeMap.items.pushBack(model::ItemInstance{
        .itemTemplateName = "TestBeer",
        .x = 1,
        .y = 0,
    });
    const auto message = game::formatExamineMessage(map, activeMap, 1, 0, 1, 0, database);
    ok = assertEqualStr(message, "Examine:\ngrass\nFriendly NPC\nTest Beer",
                        "examine message with character and item") &&
         ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    auto map = makeMap(3, 3);
    state.mapInstances[map.templateName] = std::move(map);

    model::MapGridTemplate grid;
    grid.name = "test_grid";
    grid.gridWidth = 1;
    grid.gridHeight = 1;
    grid.mapWidth = 3;
    grid.mapHeight = 3;
    grid.cells = {{"test_map"}};
    database.addMapGridTemplate(grid);
    state.world.activeMap.gridId = "test_grid";
    state.world.activeMap.mapLayer = 0;

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    auto character = model::CharacterInstance{};
    character.id = "player1";
    character.x = 1;
    character.y = 1;
    state.world.activeMap.characters.pushBack(character);

    auto& destMap = state.mapInstances["test_map"];
    auto& destTile =
        model::mapLayerAt(model::mapInstanceTiles(destMap), 0)[static_cast<size_t>(1 * 3 + 2)];
    destTile.eventTrigger =
        model::TileEventTrigger{.eventId = "on_step", .requiresLook = false};

    state::actions::WorldMovePlayer moveEast(1, 0);
    moveEast.execute(&state);

    ok = assertTrue(state.triggers.pendingSpecialEventId.has_value(),
                    "move queues step event") &&
         ok;
    ok = assertEqualStr(*state.triggers.pendingSpecialEventId, "on_step",
                        "step event id after move") &&
         ok;
    ok = assertTrue(state.world.activeMap.characters[0].x == 2 &&
                        state.world.activeMap.characters[0].y == 1,
                    "avatar moved east") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestTileTriggers failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestTileTriggers completed successfully" << LOG_ENDL;
  return 0;
}
