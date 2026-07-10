#include "state/WorldUpdater.h"
#include "model/instances/World.h"
#include "state/State.h"

namespace state {

namespace {

const model::CharacterInstance* findCharacterById(const model::MapInstance& map,
                                                  const bmin::String& id) {
  for (size_t i = 0; i < map.characters.size(); i++) {
    if (map.characters[i].id == id) {
      return &map.characters[i];
    }
  }
  return nullptr;
}

bmin::String resolveFollowCharacterId(const State& state) {
  if (!state.world.cameraFollowCharacterId.empty()) {
    return state.world.cameraFollowCharacterId;
  }

  const auto& player = state.player;
  if (player.party.empty()) {
    return bmin::String{};
  }

  auto partyIndex = player.currentPartyMemberIndex;
  if (partyIndex < 0 ||
      static_cast<size_t>(partyIndex) >= player.party.size()) {
    partyIndex = 0;
  }
  return player.party[static_cast<size_t>(partyIndex)].instanceId;
}

} // namespace

void worldUpdate(State& state, int /*dt*/) {
  auto& world = state.world;
  if (world.cameraMode != model::CameraMode::Follow) {
    return;
  }
  if (world.viewW <= 0 || world.viewH <= 0) {
    return;
  }

  auto followId = resolveFollowCharacterId(state);
  if (followId.empty()) {
    return;
  }

  const auto* target = findCharacterById(world.currentMap, followId);
  if (!target) {
    return;
  }

  auto cam = model::computeCameraFollow(
      target->x, target->y, world.currentMap, world.viewW, world.viewH);
  world.camX = cam.camX;
  world.camY = cam.camY;
}

} // namespace state
