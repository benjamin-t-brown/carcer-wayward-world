#include "db/Database.h"
#include "game/map/Camera.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/MapGrids.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/combat/SetActiveCombatCharacter.hpp"
#include "actions/world/WorldMoveActionAim.hpp"
#include "actions/world/WorldSetActionAim.hpp"
#include "actions/world/WorldSetActionMode.hpp"

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

void setupGrid(db::Database& database, state::State& state, int width, int height) {
  auto map = model::MapInstance{};
  map.id = "test_map";
  map.templateName = "test_map";
  map.width = width;
  map.height = height;
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "test_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = width;
  grid.mapHeight = height;
  grid.cells = {{"test_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "test_grid";
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

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldActionAim" << LOG_ENDL;
  auto ok = true;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupGrid(database, state, 5, 5);
    state.world.camera.cameraMode = model::CameraMode::Follow;
    placeAvatar(state, 2, 2);

    state::actions::WorldSetActionMode setExamine(model::WorldActionMode::EXAMINE);
    setExamine.execute(&state);

    ok = assertTrue(state.world.actionMode == model::WorldActionMode::EXAMINE,
                    "examine mode set") &&
         ok;
    ok = assertTrue(state.world.actionAimTile.has_value(), "aim initialized") && ok;
    if (state.world.actionAimTile) {
      ok = assertEqual(state.world.actionAimTile->x, 2, "aim x under avatar") && ok;
      ok = assertEqual(state.world.actionAimTile->y, 2, "aim y under avatar") && ok;
    }
    ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Follow,
                    "camera stays Follow after enter") &&
         ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupGrid(database, state, 5, 5);
    state.world.camera.cameraMode = model::CameraMode::Follow;
    placeAvatar(state, 2, 2);

    state::actions::WorldSetActionMode setTalk(model::WorldActionMode::TALK);
    setTalk.execute(&state);
    state::actions::WorldSetActionMode clear(model::WorldActionMode::NONE);
    clear.execute(&state);

    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "mode cleared") &&
         ok;
    ok = assertFalse(state.world.actionAimTile.has_value(), "aim cleared on NONE") &&
         ok;
    ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Follow,
                    "camera stays Follow after clear") &&
         ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupGrid(database, state, 5, 5);
    state.world.actionMode = model::WorldActionMode::EXAMINE;
    state.world.actionAimTile = model::TileXY{0, 0};

    state::actions::WorldMoveActionAim moveWest(-1, 0);
    moveWest.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 0, "clamp west x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 0, "clamp west y") && ok;

    state::actions::WorldMoveActionAim moveSouthEast(1, 1);
    moveSouthEast.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 1, "move se x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 1, "move se y") && ok;

    state.world.actionAimTile = model::TileXY{4, 4};
    state::actions::WorldMoveActionAim moveEast(1, 0);
    moveEast.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 4, "clamp east x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 4, "clamp east y") && ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupGrid(database, state, 5, 5);
    state.world.actionMode = model::WorldActionMode::TALK;
    state.world.actionAimTile = model::TileXY{1, 1};

    state::actions::WorldSetActionAim setAim(3, 4);
    setAim.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 3, "absolute aim x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 4, "absolute aim y") && ok;

    state::actions::WorldSetActionAim clampAim(99, -5);
    clampAim.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 4, "absolute clamp x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 0, "absolute clamp y") && ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupGrid(database, state, 5, 5);
    state.world.actionMode = model::WorldActionMode::NONE;
    state.world.actionAimTile.reset();

    state::actions::WorldMoveActionAim move(1, 0);
    move.execute(&state);
    ok = assertFalse(state.world.actionAimTile.has_value(),
                     "move aim no-op when mode NONE") &&
         ok;

    state::actions::WorldSetActionAim setAim(2, 2);
    setAim.execute(&state);
    ok = assertFalse(state.world.actionAimTile.has_value(),
                     "set aim no-op when mode NONE") &&
         ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupGrid(database, state, 5, 5);
    placeAvatar(state, 2, 2);
    state.world.camera.viewW = 100;
    state.world.camera.viewH = 80;
    state.world.camera.camX = 999;
    state.world.camera.camY = 999;
    state.world.camera.cameraMode = model::CameraMode::Follow;

    state::actions::WorldSetActionMode setSpell(model::WorldActionMode::SPELL);
    setSpell.execute(&state);

    ok = assertTrue(state.world.actionMode == model::WorldActionMode::SPELL,
                    "spell mode set") &&
         ok;
    ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Aiming,
                    "spell aim sets Aiming") &&
         ok;
    ok = assertTrue(state.world.actionAimTile.has_value(), "spell aim initialized") &&
         ok;
    if (state.world.actionAimTile) {
      ok = assertEqual(state.world.actionAimTile->x, 2, "spell aim x") && ok;
      ok = assertEqual(state.world.actionAimTile->y, 2, "spell aim y") && ok;
    }
    auto expected = game::computeCameraFollow(2, 2, 100, 80);
    ok = assertEqual(state.world.camera.camX, expected.camX, "spell enter camX") && ok;
    ok = assertEqual(state.world.camera.camY, expected.camY, "spell enter camY") && ok;

    state::actions::WorldMoveActionAim moveEast(1, 0);
    moveEast.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 3, "keyboard aim x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 2, "keyboard aim y") && ok;
    expected = game::computeCameraFollow(3, 2, 100, 80);
    ok = assertEqual(state.world.camera.camX, expected.camX, "keyboard aim camX") && ok;
    ok = assertEqual(state.world.camera.camY, expected.camY, "keyboard aim camY") && ok;

    const auto keyboardCamX = state.world.camera.camX;
    const auto keyboardCamY = state.world.camera.camY;
    state::actions::WorldSetActionAim setAim(0, 1);
    setAim.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 0, "mouse aim x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 1, "mouse aim y") && ok;
    ok = assertEqual(state.world.camera.camX, keyboardCamX, "mouse aim keeps camX") &&
         ok;
    ok = assertEqual(state.world.camera.camY, keyboardCamY, "mouse aim keeps camY") &&
         ok;

    state::actions::WorldSetActionMode clear(model::WorldActionMode::NONE);
    clear.execute(&state);
    ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Follow,
                    "town spell cancel restores Follow") &&
         ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupGrid(database, state, 5, 5);
    placeAvatar(state, 2, 2);
    state.world.combat.active = true;
    state.world.combat.activeCharacterId = "player1";
    state.world.camera.viewW = 100;
    state.world.camera.viewH = 80;
    state.world.camera.cameraMode = model::CameraMode::Follow;

    state::actions::WorldSetActionMode setSpell(model::WorldActionMode::SPELL);
    setSpell.execute(&state);
    state::actions::WorldMoveActionAim moveEast(1, 0);
    moveEast.execute(&state);
    auto aimed = game::computeCameraFollow(3, 2, 100, 80);
    ok = assertEqual(state.world.camera.camX, aimed.camX, "combat aim camX") && ok;
    ok = assertEqual(state.world.camera.camY, aimed.camY, "combat aim camY") && ok;

    state::actions::WorldSetActionMode clear(model::WorldActionMode::NONE);
    clear.execute(&state);
    ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Aiming,
                    "combat spell end keeps Aiming") &&
         ok;
    ok = assertEqual(state.world.camera.camX, aimed.camX, "combat spell end keeps camX") &&
         ok;
    ok = assertEqual(state.world.camera.camY, aimed.camY, "combat spell end keeps camY") &&
         ok;

    state::actions::SetActiveCombatCharacter nextTurn("player1");
    nextTurn.execute(&state);
    auto nextTurnCam = game::computeCameraFollow(2, 2, 100, 80);
    ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Follow,
                    "next turn restores Follow") &&
         ok;
    ok = assertEqual(state.world.camera.camX, nextTurnCam.camX, "next turn camX") && ok;
    ok = assertEqual(state.world.camera.camY, nextTurnCam.camY, "next turn camY") && ok;
  }

  {
    auto& state = stateManager.getState();
    state = state::State{};
    setupGrid(database, state, 5, 5);
    placeAvatar(state, 2, 2);
    state.world.camera.viewW = 100;
    state.world.camera.viewH = 80;
    auto expected = game::computeCameraFollow(2, 2, 100, 80);
    state.world.camera.camX = expected.camX;
    state.world.camera.camY = expected.camY;
    state.world.camera.cameraMode = model::CameraMode::Follow;

    state::actions::WorldSetActionMode setExamine(model::WorldActionMode::EXAMINE);
    setExamine.execute(&state);
    state::actions::WorldMoveActionAim moveEast(1, 0);
    moveEast.execute(&state);
    ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Follow,
                    "examine camera stays Follow") &&
         ok;
    ok = assertEqual(state.world.actionAimTile->x, 3, "examine aim x") && ok;
    ok = assertEqual(state.world.camera.camX, expected.camX, "examine move keeps camX") &&
         ok;
    ok = assertEqual(state.world.camera.camY, expected.camY, "examine move keeps camY") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestWorldActionAim failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestWorldActionAim completed successfully" << LOG_ENDL;
  return 0;
}
