module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <cmath>

module carcer.world_updater;
import carcer.actions;
import carcer.data;
import carcer.game.map;
import carcer.model;
import carcer.ui.helpers;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace state {

namespace {

bmin::String resolveFollowCharacterId(const State& state) {
  if (!state.world.camera.cameraFollowCharacterId.empty()) {
    return state.world.camera.cameraFollowCharacterId;
  }

  const auto& player = state.player;
  if (player.party.empty()) {
    return bmin::String{};
  }

  // Default follow target is the party leader avatar used for town movement.
  return player.party[0].instanceId;
}

void enqueueCpuCombatTurn(StateManager& stateManager) {
  auto& state = stateManager.getState();
  auto& combat = state.world.combat;
  if (combat.activeCharacterId.empty() ||
      model::isPartyMember(state.player, combat.activeCharacterId)) {
    return;
  }
  game::ActiveMapOrchestrator activeMap(
      state.world.activeMap, state.mapInstances, nullptr);
  const auto* character = activeMap.findCharacterById(combat.activeCharacterId);
  if (character == nullptr) {
    return;
  }

  combat.isWaitingForAction = false;
  stateManager.enqueueAction(
      stateManager.getActionData(), new actions::DoCPUCombatTurn(), 0);
}

void updateDamageParticles(model::World& world, sdl2w::Window* window, int deltaTimeMs) {
  if (world.activeMap.damageParticles.empty() || deltaTimeMs <= 0) {
    return;
  }

  if (!window) {
    return;
  }
  auto& store = window->getStore();

  for (size_t i = 0; i < world.activeMap.damageParticles.size();) {
    auto& particle = world.activeMap.damageParticles[i];

    /* animation via MapView */

    timerStructUpdate(particle.lifetime, deltaTimeMs);
    if (timerStructIsComplete(particle.lifetime)) {
      world.activeMap.damageParticles.erase(i);
    } else {
      ++i;
    }
  }
}

void updateProjectiles(model::World& world, int deltaTimeMs) {
  if (world.activeMap.projectiles.empty() || deltaTimeMs <= 0) {
    return;
  }
  static const double tileHeight = 32;
  static const double pi = 3.14159265358979323846;

  for (size_t i = 0; i < world.activeMap.projectiles.size();) {
    auto& projectile = world.activeMap.projectiles[i];
    timerStructUpdate(projectile.travel, deltaTimeMs);

    if (timerStructIsComplete(projectile.travel)) {
      world.activeMap.projectiles.erase(i);
    } else {
      auto pct = timerStructGetPct(projectile.travel);
      switch (projectile.projectilePath) {
      case model::ProjectilePath::PROJECTILE_PATH_SHORT:
        projectile.yOffset = (tileHeight) * sin(pct * pi);
        break;
      case model::ProjectilePath::PROJECTILE_PATH_MEDIUM:
        projectile.yOffset = (tileHeight * 1.5) * sin(pct * pi);
        break;
      case model::ProjectilePath::PROJECTILE_PATH_TALL:
        projectile.yOffset = (tileHeight * 3) * sin(pct * pi);
        break;
      case model::ProjectilePath::PROJECTILE_PATH_NONE:
        projectile.yOffset = 0;
        break;
      }
      ++i;
    }
  }
}

} // namespace

void worldUpdate(sdl2w::Window* window, StateManager& stateManager, int dt) {
  auto& state = stateManager.getState();
  if (window != nullptr) {
    for (const auto& soundName : state.soundsToPlay) {
      if (soundName.empty()) {
        continue;
      }
      window->playSound(bmin::toStringView(soundName));
    }
  }
  state.soundsToPlay.clear();
  updateDamageParticles(state.world, window, dt);
  updateProjectiles(state.world, dt);
  game::ActiveMapOrchestrator activeMap(
      state.world.activeMap, state.mapInstances, nullptr);

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
  if (const auto* followTarget = activeMap.findCharacterById(followId)) {
    auto cam = game::computeCameraFollow(
        followTarget->x, followTarget->y, world.camera.viewW, world.camera.viewH);
    world.camera.camX = cam.camX;
    world.camera.camY = cam.camY;
  }
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
