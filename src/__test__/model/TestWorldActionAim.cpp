#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/State.h"
#include "state/actions/world/WorldMoveActionAim.hpp"
#include "state/actions/world/WorldSetActionAim.hpp"
#include "state/actions/world/WorldSetActionMode.hpp"

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

model::MapInstance makeMap(int width, int height) {
  auto map = model::MapInstance{};
  map.width = width;
  map.height = height;
  return map;
}

void placeAvatar(state::State& state, int x, int y) {
  auto member = model::CharacterPlayer{};
  member.instanceId = "player1";
  state.player.party.pushBack(member);
  state.world.currentMap.characters.pushBack(model::CharacterInstance{
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

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
    state.world.cameraMode = model::CameraMode::Follow;
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
    ok = assertTrue(state.world.cameraMode == model::CameraMode::Follow,
                    "camera stays Follow after enter") &&
         ok;
  }

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
    state.world.cameraMode = model::CameraMode::Follow;
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
    ok = assertTrue(state.world.cameraMode == model::CameraMode::Follow,
                    "camera stays Follow after clear") &&
         ok;
  }

  {
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
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
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
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
    state::State state{};
    state.world.currentMap = makeMap(5, 5);
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

  if (!ok) {
    LOG(ERROR) << "TestWorldActionAim failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestWorldActionAim completed successfully" << LOG_ENDL;
  return 0;
}
