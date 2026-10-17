#include "db/Database.h"
#include "game/map/MapWalkability.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/world/WorldInteractAt.hpp"
#include "in3/EventRunnerHelpers.h"
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

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
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

bool soundsContain(const state::State& state, const char* name) {
  for (size_t i = 0; i < state.soundsToPlay.size(); i++) {
    if (state.soundsToPlay[i] == name) {
      return true;
    }
  }
  return false;
}

model::TileMetadata makeMeta(int id, bool walkable, bool isDoor) {
  auto meta = model::TileMetadata{};
  meta.id = id;
  meta.isWalkable = walkable;
  meta.isDoor = isDoor;
  meta.isSeeThrough = walkable;
  return meta;
}

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  // 0 floor walkable, 1 wall, 2 closed door, 3 open door
  tileset.tiles.pushBack(makeMeta(0, true, false));
  tileset.tiles.pushBack(makeMeta(1, false, false));
  tileset.tiles.pushBack(makeMeta(2, false, true));
  tileset.tiles.pushBack(makeMeta(3, true, true));
  database.addTilesetTemplate(tileset);
}

model::TileInstance makeTile(int x, int y, int tileId, bool visible = true) {
  auto tile = model::TileInstance{};
  tile.x = x;
  tile.y = y;
  tile.tilesetName = "test_terrain";
  tile.tileId = tileId;
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
      layer.pushBack(makeTile(x, y, 0, visible));
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

void placeAvatar(state::State& state, int x, int y) {
  auto member = model::CharacterPlayer{};
  member.instanceId = "player1";
  state.player.party.pushBack(member);
  state.world.activeMap.characters.pushBack(model::CharacterInstance{
      .id = "player1",
      .templateName = "Hero",
      .x = x,
      .y = y,
  });
}

model::MapInstance& mapOf(state::State& state) {
  return state.mapInstances["test_map"];
}

model::TileInstance* tileAt(model::MapInstance& map, int x, int y) {
  return &model::mapLayerAt(model::mapInstanceTiles(map), 0)[static_cast<size_t>(y * map.width +
                                                                                x)];
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldInteractAt" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  addTestTileset(database);

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  bool ok = true;

  // Non-adjacent aim: clears mode, no event / door change
  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    placeAvatar(state, 2, 2);
    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{4, 4};

    auto& tile = *tileAt(mapOf(state), 4, 4);
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "far_use",
        .requiresUse = true,
    };

    state::actions::WorldInteractAt interact(4, 4);
    interact.execute(&state);

    ok = assertFalse(state.triggers.pendingSpecialEventId.has_value(),
                     "non-adjacent does not queue use event") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "non-adjacent clears interact mode") &&
         ok;
    ok = assertFalse(state.world.actionAimTile.has_value(),
                     "non-adjacent clears aim") &&
         ok;
    ok = assertEqual(static_cast<int>(state.uiState.floatingNotifications.size()), 1,
                     "non-adjacent shows too-far notification") &&
         ok;
    if (!state.uiState.floatingNotifications.empty()) {
      ok = assertEqualStr(state.uiState.floatingNotifications[0].message,
                          "Interact: Too far.", "non-adjacent notification text") &&
           ok;
    }
  }

  // Diagonal adjacency is allowed (Chebyshev ≤ 1)
  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    placeAvatar(state, 2, 2);
    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{3, 3};

    auto& tile = *tileAt(mapOf(state), 3, 3);
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "diag_use",
        .requiresUse = true,
    };

    state::actions::WorldInteractAt interact(3, 3);
    interact.execute(&state);

    ok = assertTrue(state.triggers.pendingSpecialEventId.has_value(),
                    "diagonal adjacent queues use event") &&
         ok;
    if (state.triggers.pendingSpecialEventId) {
      ok = assertEqualStr(*state.triggers.pendingSpecialEventId, "diag_use",
                          "diagonal use event id") &&
           ok;
    }
  }

  // requiresUse trigger path + sound + mode clear
  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    placeAvatar(state, 2, 2);
    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto& tile = *tileAt(mapOf(state), 3, 2);
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "use_event",
        .requiresUse = true,
    };

    state::actions::WorldInteractAt interact(3, 2);
    interact.execute(&state);

    ok = assertTrue(state.triggers.pendingSpecialEventId.has_value(),
                    "use trigger queues event") &&
         ok;
    if (state.triggers.pendingSpecialEventId) {
      ok = assertEqualStr(*state.triggers.pendingSpecialEventId, "use_event",
                          "use event id") &&
           ok;
    }
    ok = assertTrue(soundsContain(state, "event"),
                    "use trigger plays default activationSound") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "interact mode cleared after use trigger") &&
         ok;
    ok = assertFalse(state.world.actionAimTile.has_value(),
                     "aim cleared after use trigger") &&
         ok;
  }

  // Condition gate on requiresUse
  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    placeAvatar(state, 2, 2);
    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto& tile = *tileAt(mapOf(state), 3, 2);
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "gated_use",
        .requiresUse = true,
        .condition = "IS(flag)",
    };

    state::actions::WorldInteractAt blocked(3, 2);
    blocked.execute(&state);
    ok = assertFalse(state.triggers.pendingSpecialEventId.has_value(),
                     "use condition false does not queue event") &&
         ok;

    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{3, 2};
    in3::setStorage(state.specialEventStorage, "flag", "1");
    state::actions::WorldInteractAt allowed(3, 2);
    allowed.execute(&state);
    ok = assertTrue(state.triggers.pendingSpecialEventId.has_value(),
                    "use condition true queues event") &&
         ok;
    if (state.triggers.pendingSpecialEventId) {
      ok = assertEqualStr(*state.triggers.pendingSpecialEventId, "gated_use",
                          "gated use event id") &&
           ok;
    }
  }

  // Close open door + remove openedDoors persistence
  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    placeAvatar(state, 2, 2);
    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto* door = tileAt(mapOf(state), 3, 2);
    door->tileId = 3;
    game::upsertOpenedDoor(mapOf(state).persistentState.openedDoors, 0, 3, 2, 3);

    state::actions::WorldInteractAt interact(3, 2);
    interact.execute(&state);

    ok = assertEqual(door->tileId, 2, "open door closed to tileId-1") && ok;
    ok = assertTrue(game::isClosedDoorTile(*door, database), "closed door metadata") &&
         ok;
    ok = assertTrue(mapOf(state).persistentState.openedDoors.empty(),
                    "openedDoors cleared on close") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "interact mode cleared after door close") &&
         ok;

    // Re-apply persistence: door stays closed (no open record)
    auto restored = makeMap(5, 5);
    auto* restoredDoor = tileAt(restored, 3, 2);
    restoredDoor->tileId = 2;
    game::applyOpenedDoors(restored, mapOf(state).persistentState.openedDoors);
    ok = assertEqual(restoredDoor->tileId, 2, "door stays closed after applyOpenedDoors") &&
         ok;
  }

  // Occupied open door refuses close
  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    placeAvatar(state, 2, 2);
    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto* door = tileAt(mapOf(state), 3, 2);
    door->tileId = 3;
    game::upsertOpenedDoor(mapOf(state).persistentState.openedDoors, 0, 3, 2, 3);

    state.world.activeMap.characters.pushBack(model::CharacterInstance{
        .id = "blocker",
        .templateName = "Npc",
        .x = 3,
        .y = 2,
    });

    state::actions::WorldInteractAt interact(3, 2);
    interact.execute(&state);

    ok = assertEqual(door->tileId, 3, "occupied door stays open") && ok;
    ok = assertEqual(static_cast<int>(mapOf(state).persistentState.openedDoors.size()),
                     1,
                     "occupied close keeps openedDoors") &&
         ok;
  }

  // Standing requiresAction travel
  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state);
    placeAvatar(state, 2, 2);
    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{2, 2};

    auto& tile = *tileAt(mapOf(state), 2, 2);
    tile.travelTrigger = model::TravelTrigger{
        .destinationMapName = "action_dest",
        .requiresAction = true,
    };

    state::actions::WorldInteractAt interact(2, 2);
    interact.execute(&state);

    ok = assertTrue(state.triggers.pendingTravel.has_value(),
                    "standing interact queues action travel") &&
         ok;
    if (state.triggers.pendingTravel) {
      ok = assertEqualStr(state.triggers.pendingTravel->destinationMapName, "action_dest",
                          "standing travel map") &&
           ok;
    }
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "interact mode cleared after standing travel") &&
         ok;
  }

  // Non-visible target keeps aim (same as Examine)
  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupState(database, state, false);
    placeAvatar(state, 2, 2);
    state.world.actionMode = model::WorldActionMode::INTERACT;
    state.world.actionAimTile = model::TileXY{3, 2};

    auto& tile = *tileAt(mapOf(state), 3, 2);
    tile.eventTrigger = model::TileEventTrigger{
        .eventId = "hidden_use",
        .requiresUse = true,
    };

    ok = assertFalse(game::isTileCurrentlyVisible(mapOf(state), 3, 2),
                     "target not visible") &&
         ok;

    state::actions::WorldInteractAt interact(3, 2);
    interact.execute(&state);

    ok = assertFalse(state.triggers.pendingSpecialEventId.has_value(),
                     "non-visible does not queue use event") &&
         ok;
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::INTERACT,
                    "interact mode kept when not visible") &&
         ok;
    ok = assertTrue(state.world.actionAimTile.has_value() &&
                        state.world.actionAimTile->x == 3 &&
                        state.world.actionAimTile->y == 2,
                    "aim kept when not visible") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestWorldInteractAt failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestWorldInteractAt completed successfully" << LOG_ENDL;
  return 0;
}
