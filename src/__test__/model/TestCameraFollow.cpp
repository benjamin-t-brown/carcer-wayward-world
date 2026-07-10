#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/State.h"
#include "state/WorldUpdater.h"
#include "bmin/String.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertTrue(bool condition, const char* label) {
  if (!condition) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestCameraFollow" << LOG_ENDL;

  bool ok = true;

  // Pure helper: free center (no map-bound clamp)
  {
    auto map = model::MapInstance{};
    map.width = 10;
    map.height = 10;
    map.spriteWidth = 28;
    map.spriteHeight = 32;
    auto viewW = 100;
    auto viewH = 80;

    // Target near origin → negative cam so sprite stays centered
    auto camNear = model::computeCameraFollow(0, 0, map, viewW, viewH);
    ok = assertEqual(camNear.camX, 0 * 28 - viewW / 2 + 28 / 2, "near.camX") && ok;
    ok = assertEqual(camNear.camY, 0 * 32 - viewH / 2 + 32 / 2, "near.camY") && ok;

    // Target at (5,5)
    auto camMid = model::computeCameraFollow(5, 5, map, viewW, viewH);
    ok = assertEqual(camMid.camX, 5 * 28 - viewW / 2 + 28 / 2, "mid.camX") && ok;
    ok = assertEqual(camMid.camY, 5 * 32 - viewH / 2 + 32 / 2, "mid.camY") && ok;

    // Target at far corner → cam may exceed map size (outside visible)
    auto camFar = model::computeCameraFollow(9, 9, map, viewW, viewH);
    ok = assertEqual(camFar.camX, 9 * 28 - viewW / 2 + 28 / 2, "far.camX") && ok;
    ok = assertEqual(camFar.camY, 9 * 32 - viewH / 2 + 32 / 2, "far.camY") && ok;
  }

  // worldUpdate snap when Follow mode + follow id + view dims set
  {
    state::State state;
    ok = assertTrue(state.world.cameraMode == model::CameraMode::Follow,
                    "default cameraMode Follow") &&
         ok;
    state.world.currentMap.width = 10;
    state.world.currentMap.height = 10;
    state.world.currentMap.spriteWidth = 28;
    state.world.currentMap.spriteHeight = 32;
    state.world.viewW = 100;
    state.world.viewH = 80;
    state.world.camX = 999;
    state.world.camY = 999;

    auto character = model::CharacterInstance{};
    character.id = "follow-me";
    character.x = 5;
    character.y = 5;
    state.world.currentMap.characters.pushBack(std::move(character));
    state.world.cameraFollowCharacterId = "follow-me";

    state::worldUpdate(state, 16);

    auto expected = model::computeCameraFollow(
        5, 5, state.world.currentMap, state.world.viewW, state.world.viewH);
    ok = assertEqual(state.world.camX, expected.camX, "worldUpdate.camX") && ok;
    ok = assertEqual(state.world.camY, expected.camY, "worldUpdate.camY") && ok;
  }

  // Follow mode with empty follow id → auto-resolve party member avatar
  {
    state::State state;
    state.world.currentMap.width = 10;
    state.world.currentMap.height = 10;
    state.world.currentMap.spriteWidth = 28;
    state.world.currentMap.spriteHeight = 32;
    state.world.viewW = 100;
    state.world.viewH = 80;
    state.world.camX = 999;
    state.world.camY = 999;

    auto member = model::CharacterPlayer{};
    member.instanceId = "party-avatar";
    state.player.party.pushBack(std::move(member));
    state.player.currentPartyMemberIndex = 0;

    auto character = model::CharacterInstance{};
    character.id = "party-avatar";
    character.x = 5;
    character.y = 5;
    state.world.currentMap.characters.pushBack(std::move(character));

    state::worldUpdate(state, 16);

    auto expected = model::computeCameraFollow(
        5, 5, state.world.currentMap, state.world.viewW, state.world.viewH);
    ok = assertEqual(state.world.camX, expected.camX, "autoResolve.camX") && ok;
    ok = assertEqual(state.world.camY, expected.camY, "autoResolve.camY") && ok;
  }

  // Aiming / Dragging / Controlled leave cam unchanged
  {
    const model::CameraMode modes[] = {model::CameraMode::Aiming,
                                       model::CameraMode::Dragging,
                                       model::CameraMode::Controlled};
    const char* labels[] = {"Aiming", "Dragging", "Controlled"};

    for (size_t mi = 0; mi < 3; mi++) {
      state::State state;
      state.world.cameraMode = modes[mi];
      state.world.currentMap.width = 10;
      state.world.currentMap.height = 10;
      state.world.currentMap.spriteWidth = 28;
      state.world.currentMap.spriteHeight = 32;
      state.world.viewW = 100;
      state.world.viewH = 80;
      state.world.camX = 42;
      state.world.camY = 43;

      auto character = model::CharacterInstance{};
      character.id = "follow-me";
      character.x = 5;
      character.y = 5;
      state.world.currentMap.characters.pushBack(std::move(character));
      state.world.cameraFollowCharacterId = "follow-me";

      state::worldUpdate(state, 16);

      auto labelX = bmin::String(labels[mi]) + ".camX";
      auto labelY = bmin::String(labels[mi]) + ".camY";
      ok = assertEqual(state.world.camX, 42, labelX.c_str()) && ok;
      ok = assertEqual(state.world.camY, 43, labelY.c_str()) && ok;
    }
  }

  // Missing target / viewW<=0 → no-op
  {
    state::State state;
    state.world.currentMap.width = 10;
    state.world.currentMap.height = 10;
    state.world.currentMap.spriteWidth = 28;
    state.world.currentMap.spriteHeight = 32;
    state.world.viewW = 0;
    state.world.viewH = 80;
    state.world.camX = 42;
    state.world.camY = 43;
    state.world.cameraFollowCharacterId = "missing";
    state::worldUpdate(state, 0);
    ok = assertEqual(state.world.camX, 42, "noop viewW.camX") && ok;
    ok = assertEqual(state.world.camY, 43, "noop viewW.camY") && ok;

    state.world.viewW = 100;
    state::worldUpdate(state, 0);
    ok = assertEqual(state.world.camX, 42, "noop missing.camX") && ok;
    ok = assertEqual(state.world.camY, 43, "noop missing.camY") && ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestCameraFollow assertions failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestCameraFollow completed successfully" << LOG_ENDL;
  return 0;
}
