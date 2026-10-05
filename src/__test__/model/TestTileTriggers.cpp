#include "db/Database.h"
#include "game/map/TileTriggers.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/world/WorldMovePlayer.hpp"
#include "bmin/Map.h"
#include "bmin/String.h"
#include "in3/EventRunnerHelpers.h"

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

    auto storage = bmin::Map<bmin::String, bmin::String>{};
    const auto result = game::resolveStepTriggersAt(map, 0, 0, storage);
    ok = assertTrue(result.specialEventId.has_value(), "event takes precedence") &&
         ok;
    ok = assertEqualStr(*result.specialEventId, "step_event", "event id") && ok;
    ok = assertTrue(!result.travel.has_value(),
                    "travel ignored when event present") &&
         ok;
  }

  {
    auto map = makeMap(2, 2);
    model::mapLayerAt(model::mapInstanceTiles(map), 0)[0].travelTrigger =
        model::TravelTrigger{
            .destinationMapName = "dest_map",
            .destinationX = 3,
            .destinationY = 4,
        };

    auto storage = bmin::Map<bmin::String, bmin::String>{};
    const auto result = game::resolveStepTriggersAt(map, 0, 0, storage);
    ok = assertTrue(!result.specialEventId.has_value(), "no event pending") && ok;
    ok = assertTrue(result.travel.has_value(), "travel pending") && ok;
    ok = assertEqualStr(result.travel->destinationMapName, "dest_map",
                        "travel map") &&
         ok;
  }

  {
    auto map = makeMap(2, 2);
    model::mapLayerAt(model::mapInstanceTiles(map), 0)[0].travelTrigger =
        model::TravelTrigger{
            .destinationMapName = "action_dest",
            .requiresAction = true,
        };

    auto storage = bmin::Map<bmin::String, bmin::String>{};
    const auto stepResult = game::resolveStepTriggersAt(map, 0, 0, storage);
    ok = assertTrue(!stepResult.travel.has_value(),
                    "action travel not queued on step") &&
         ok;

    const auto actionTravel = game::resolveActionTravelAtStanding(map, 0, 0);
    ok = assertTrue(actionTravel.has_value(),
                    "action travel queued on interact") &&
         ok;
    ok = assertEqualStr(actionTravel->destinationMapName, "action_dest",
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

  {
    auto map = makeMap(2, 2);
    auto& tile = model::mapLayerAt(model::mapInstanceTiles(map), 0)[0];
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "gated_event",
        .requiresLook = false,
        .condition = "IS(flag)",
    };
    tile.travelTrigger = model::TravelTrigger{
        .destinationMapName = "fallback",
    };

    auto storage = bmin::Map<bmin::String, bmin::String>{};
    const auto blocked = game::resolveStepTriggersAt(map, 0, 0, storage);
    ok = assertTrue(!blocked.specialEventId.has_value(),
                    "false condition does not run the event") &&
         ok;
    ok = assertTrue(blocked.travel.has_value(),
                    "travel still runs when the event condition is false") &&
         ok;

    in3::setStorage(storage, "flag", "1");
    const auto allowed = game::resolveStepTriggersAt(map, 0, 0, storage);
    ok = assertTrue(allowed.specialEventId.has_value(),
                    "true condition runs the event") &&
         ok;
    ok = assertEqualStr(*allowed.specialEventId, "gated_event",
                        "gated event id") &&
         ok;
    ok = assertTrue(!allowed.travel.has_value(),
                    "travel ignored when the event condition passes") &&
         ok;
  }

  {
    auto map = makeMap(2, 2);
    auto& tile = model::mapLayerAt(model::mapInstanceTiles(map), 0)[0];
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "once_event",
        .requiresLook = false,
        .condition = "ONCE(step)",
    };

    auto storage = bmin::Map<bmin::String, bmin::String>{};
    const auto first = game::resolveStepTriggersAt(map, 0, 0, storage);
    ok = assertTrue(first.specialEventId.has_value(), "ONCE runs the first time") &&
         ok;
    ok = assertEqualStr(in3::getStorage(storage, "once.step").value_or(""),
                        "true",
                        "ONCE commits its key") &&
         ok;

    const auto second = game::resolveStepTriggersAt(map, 0, 0, storage);
    ok = assertTrue(!second.specialEventId.has_value(),
                    "ONCE does not run again") &&
         ok;
  }

  {
    auto trigger = model::TileEventTrigger{
        .eventId = "overlay_event",
        .overlayVisibility = model::TileOverlayVisibility::SHOW_EVENT_ON_TILE,
        .condition = "ONCE(sign)",
    };
    auto storage = bmin::Map<bmin::String, bmin::String>{};
    ok = assertTrue(game::tileEventConditionHolds(trigger, storage),
                    "soft ONCE is true before the event runs") &&
         ok;
    ok = assertTrue(!in3::getStorage(storage, "once.sign").has_value(),
                    "soft ONCE does not write the key") &&
         ok;
    ok = assertTrue(game::tileEventShouldRun(trigger, storage),
                    "hard ONCE still runs after a soft check") &&
         ok;
    ok = assertEqualStr(in3::getStorage(storage, "once.sign").value_or(""),
                        "true",
                        "hard ONCE commits after the soft check") &&
         ok;
    ok = assertTrue(!game::tileEventConditionHolds(trigger, storage),
                    "soft ONCE is false after the key is committed") &&
         ok;

    trigger.condition = "IS(flag)";
    ok = assertTrue(!game::tileEventConditionHolds(trigger, storage),
                    "false IS hides the event overlay") &&
         ok;
    in3::setStorage(storage, "flag", "1");
    ok = assertTrue(game::tileEventConditionHolds(trigger, storage),
                    "true IS shows the event overlay") &&
         ok;
  }

  {
    auto map = makeMap(2, 2);
    model::mapLayerAt(model::mapInstanceTiles(map), 0)[0].eventTrigger =
        model::TileEventTrigger{
            .eventId = "bad_event",
            .requiresLook = false,
            .condition = "NOT_A_FUNC(x)",
        };

    auto storage = bmin::Map<bmin::String, bmin::String>{};
    const auto result = game::resolveStepTriggersAt(map, 0, 0, storage);
    ok = assertTrue(!result.specialEventId.has_value(),
                    "invalid condition does not run the event") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestTileTriggers failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestTileTriggers completed successfully" << LOG_ENDL;
  return 0;
}
