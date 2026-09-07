#include <functional>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string_view>
import carcer;
import sdl2w;
import bmin.string_interop;
#include "macros.h"

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

    auto setExamine = state::actions::setActionMode(model::WorldActionMode::EXAMINE);
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

    auto setTalk = state::actions::setActionMode(model::WorldActionMode::TALK);
    setTalk.execute(&state);
    auto clear = state::actions::setActionMode(model::WorldActionMode::NONE);
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

    auto moveWest = state::actions::moveActionAim(-1, 0);
    moveWest.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 0, "clamp west x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 0, "clamp west y") && ok;

    auto moveSouthEast = state::actions::moveActionAim(1, 1);
    moveSouthEast.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 1, "move se x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 1, "move se y") && ok;

    state.world.actionAimTile = model::TileXY{4, 4};
    auto moveEast = state::actions::moveActionAim(1, 0);
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

    auto setAim = state::actions::setActionAim(3, 4);
    setAim.execute(&state);
    ok = assertEqual(state.world.actionAimTile->x, 3, "absolute aim x") && ok;
    ok = assertEqual(state.world.actionAimTile->y, 4, "absolute aim y") && ok;

    auto clampAim = state::actions::setActionAim(99, -5);
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

    auto move = state::actions::moveActionAim(1, 0);
    move.execute(&state);
    ok = assertFalse(state.world.actionAimTile.has_value(),
                     "move aim no-op when mode NONE") &&
         ok;

    auto setAim = state::actions::setActionAim(2, 2);
    setAim.execute(&state);
    ok = assertFalse(state.world.actionAimTile.has_value(),
                     "set aim no-op when mode NONE") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestWorldActionAim failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestWorldActionAim completed successfully" << LOG_ENDL;
  return 0;
}
