#include "state/WorldUpdater.h"
#include "game/map/Camera.h"
#include "model/Combat.h"
#include "model/instances/World.h"
#include "sdl2w/Window.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/actions/combat/DoCPUCombatTurn.hpp"
#include "state/actions/ui/UiShowLayerSpecialEvent.hpp"
#include "state/actions/world/WorldTravel.hpp"
#include "ui/helpers/worldActions.h"

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
  if (!state.world.camera.cameraFollowCharacterId.empty()) {
    return state.world.camera.cameraFollowCharacterId;
  }

  const auto& player = state.player;
  if (player.party.empty()) {
    return bmin::String{};
  }

  auto partyIndex = player.currentPartyMemberIndex;
  if (partyIndex < 0 || static_cast<size_t>(partyIndex) >= player.party.size()) {
    partyIndex = 0;
  }
  return player.party[static_cast<size_t>(partyIndex)].instanceId;
}

void enqueueCpuCombatTurn(StateManager& stateManager) {
  auto& state = stateManager.getState();
  auto& combat = state.world.combat;
  const auto* character =
      model::findCharacterOnMap(state.world.currentMap, combat.activeCharacterId);
  if (character == nullptr || model::isPartyMember(state.player, character->id)) {
    return;
  }

  combat.isWaitingForAction = false;
  stateManager.enqueueAction(
      stateManager.getActionData(), new actions::DoCPUCombatTurn(), 0);
}

void updateDamageParticles(model::World& world, int deltaTimeMs) {
  if (world.damageParticles.empty() || deltaTimeMs <= 0) {
    return;
  }

  for (size_t i = 0; i < world.damageParticles.size();) {
    auto& particle = world.damageParticles[i];
    timerStructUpdate(particle.lifetime, deltaTimeMs);
    if (timerStructIsComplete(particle.lifetime)) {
      world.damageParticles.erase(i);
    } else {
      ++i;
    }
  }
}


} // namespace

void worldUpdate(StateManager& stateManager, int dt) {
  auto& state = stateManager.getState();
  updateDamageParticles(state.world, dt);

  auto& combat = state.world.combat;
  if (combat.active && combat.isWaitingForAction) {
    enqueueCpuCombatTurn(stateManager);
  }

  auto& world = state.world;
  if (world.camera.cameraMode != model::CameraMode::Follow) {
    return;
  }
  if (world.camera.viewW <= 0 || world.camera.viewH <= 0) {
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

  auto cam = game::computeCameraFollow(
      target->x, target->y, world.currentMap, world.camera.viewW, world.camera.viewH);
  world.camera.camX = cam.camX;
  world.camera.camY = cam.camY;
}

void worldProcessPendingTriggers(sdl2w::Window* window, StateManager& stateManager) {
  auto& state = stateManager.getState();
  bool mapChanged = false;

  if (state.triggers.pendingSpecialEventId) {
    ui::setHeldMoveActive(stateManager, false);
    auto eventId = *state.triggers.pendingSpecialEventId;
    state.triggers.pendingSpecialEventId.reset();
    state::actions::UiShowLayerSpecialEvent specialEvent =
        state::actions::UiShowLayerSpecialEvent(window, eventId);
    specialEvent.execute(&state);
  }

  if (state.triggers.pendingTravel) {
    ui::setHeldMoveActive(stateManager, false);
    auto travel = *state.triggers.pendingTravel;
    state.triggers.pendingTravel.reset();
    state::actions::WorldTravel travelAction(travel);
    travelAction.execute(&state);
    mapChanged = true;
  }

  state.triggers.mapChangedThisTick = mapChanged;
}

} // namespace state
