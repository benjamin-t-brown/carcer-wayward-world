#include "db/Database.h"
#include "game/map/MapWalkability.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/MapGrids.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/actions/world/WorldTalkAt.hpp"
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
  map.id = "test_map";
  map.templateName = "test_map";
  map.width = width;
  map.height = height;
  map.tileLayerNumber = 0;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      layer.pushBack(makeTile(x, y, visible));
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  return map;
}

void setupState(db::Database& database, state::State& state, bool visible = true) {
  auto map = makeMap(5, 5, visible);
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "test_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 5;
  grid.mapHeight = 5;
  grid.cells = {{"test_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "test_grid";
  state.world.activeMap.mapLayer = 0;
}

model::MapInstance& mapOf(state::State& state) {
  return state.mapInstances["test_map"];
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldTalkAt" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);

  auto talker = model::CharacterTemplate{};
  talker.name = "TalkNpc";
  talker.label = "Chatty One";
  talker.talk.talkName = "npc_talk_event";
  database.addCharacterTemplate(talker);

  auto silent = model::CharacterTemplate{};
  silent.name = "SilentNpc";
  silent.label = "Quiet One";
  database.addCharacterTemplate(silent);

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  bool ok = true;

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    state.world.actionMode = model::WorldActionMode::TALK;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    state.world.activeMap.characters.pushBack(model::CharacterInstance{
        .id = "player1",
        .templateName = "TalkNpc",
        .x = 2,
        .y = 2,
    });
    state.world.activeMap.characters.pushBack(model::CharacterInstance{
        .id = "npc1",
        .templateName = "TalkNpc",
        .x = 3,
        .y = 2,
    });

    state::actions::WorldTalkAt talkAt(3, 2);
    talkAt.execute(&state);

    ok = assertTrue(state.triggers.pendingSpecialEventId.has_value(),
                    "talk queues special event") &&
         ok;
    if (state.triggers.pendingSpecialEventId) {
      ok = assertEqualStr(*state.triggers.pendingSpecialEventId, "npc_talk_event",
                          "talk event id") &&
           ok;
    }
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "talk mode cleared") &&
         ok;
    ok = assertFalse(state.world.actionAimTile.has_value(), "aim cleared") && ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    state.world.actionMode = model::WorldActionMode::TALK;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    state.world.activeMap.characters.pushBack(model::CharacterInstance{
        .id = "player1",
        .templateName = "TalkNpc",
        .x = 2,
        .y = 2,
    });
    state.world.activeMap.characters.pushBack(model::CharacterInstance{
        .id = "npc2",
        .templateName = "SilentNpc",
        .x = 3,
        .y = 2,
    });

    state::actions::WorldTalkAt talkAt(3, 2);
    talkAt.execute(&state);

    ok = assertFalse(state.triggers.pendingSpecialEventId.has_value(),
                     "silent npc does not queue event") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "talk mode cleared for silent") &&
         ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    state.world.actionMode = model::WorldActionMode::TALK;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    state.world.activeMap.characters.pushBack(model::CharacterInstance{
        .id = "player1",
        .templateName = "TalkNpc",
        .x = 2,
        .y = 2,
    });

    state::actions::WorldTalkAt talkAt(3, 2);
    talkAt.execute(&state);

    ok = assertFalse(state.triggers.pendingSpecialEventId.has_value(),
                     "empty tile does not queue event") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "talk mode cleared for empty") &&
         ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state, false);
    state.world.actionMode = model::WorldActionMode::TALK;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto member = model::CharacterPlayer{};
    member.instanceId = "player1";
    state.player.party.pushBack(member);

    state.world.activeMap.characters.pushBack(model::CharacterInstance{
        .id = "player1",
        .templateName = "TalkNpc",
        .x = 2,
        .y = 2,
    });
    state.world.activeMap.characters.pushBack(model::CharacterInstance{
        .id = "npc1",
        .templateName = "TalkNpc",
        .x = 3,
        .y = 2,
    });

    ok = assertFalse(game::isTileCurrentlyVisible(mapOf(state), 3, 2),
                     "target not visible") &&
         ok;

    state::actions::WorldTalkAt talkAt(3, 2);
    talkAt.execute(&state);

    ok = assertFalse(state.triggers.pendingSpecialEventId.has_value(),
                     "non-visible does not queue event") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::TALK,
                    "talk mode kept when not visible") &&
         ok;
    ok = assertTrue(state.world.actionAimTile.has_value() &&
                        state.world.actionAimTile->x == 3 &&
                        state.world.actionAimTile->y == 2,
                    "aim kept when not visible") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestWorldTalkAt failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestWorldTalkAt completed successfully" << LOG_ENDL;
  return 0;
}
