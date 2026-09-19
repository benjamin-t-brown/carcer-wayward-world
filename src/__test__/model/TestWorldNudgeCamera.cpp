#include "game/map/Camera.h"
#include "sdl2w/Logger.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "actions/world/WorldNudgeCamera.hpp"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldNudgeCamera" << LOG_ENDL;
  auto ok = true;

  state::StateManager stateManager;
  auto& state = stateManager.getState();
  state.world.camera.camX = 100;
  state.world.camera.camY = 80;

  state::actions::WorldNudgeCamera nudgeEast(1, 0);
  nudgeEast.execute(&state);
  ok = assertEqual(state.world.camera.camX, 100 + game::kCameraTileWidth, "east camX") &&
       ok;
  ok = assertEqual(state.world.camera.camY, 80, "east camY") && ok;

  state::actions::WorldNudgeCamera nudgeNorth(0, -1);
  nudgeNorth.execute(&state);
  ok = assertEqual(state.world.camera.camX, 100 + game::kCameraTileWidth, "north camX") &&
       ok;
  ok = assertEqual(state.world.camera.camY, 80 - game::kCameraTileHeight, "north camY") &&
       ok;

  state::actions::WorldNudgeCamera nudgeWestSouth(-1, 1);
  nudgeWestSouth.execute(&state);
  ok = assertEqual(state.world.camera.camX, 100, "west-south camX") && ok;
  ok = assertEqual(state.world.camera.camY, 80, "west-south camY") && ok;

  if (!ok) {
    LOG(ERROR) << "TestWorldNudgeCamera failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestWorldNudgeCamera completed successfully" << LOG_ENDL;
  return 0;
}
