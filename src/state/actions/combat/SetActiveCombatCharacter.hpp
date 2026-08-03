#pragma once

#include "model/Combat.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/Camera.h"
#include "model/instances/World.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/world/WorldSetCamera.hpp"

namespace state {

namespace actions {

class DoCPUCombatTurn;

class SetActiveCombatCharacter : public CombatAction {
  bmin::String characterId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    auto& combat = world.combat;
    if (!combat.active) {
      return;
    }

    if (characterId.empty()) {
      if (combat.activeTurnIndex < 0 ||
          combat.activeTurnIndex >= static_cast<int>(combat.turnOrderIds.size())) {
        return;
      }
      characterId = combat.turnOrderIds[static_cast<size_t>(combat.activeTurnIndex)];
    }

    game::ActiveMapOrchestrator orch;
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }

    combat.activeCharacterId = characterId;
    combat.isWaitingForAction = true;

    world.camera.cameraFollowCharacterId = characterId;
    world.camera.cameraMode = model::CameraMode::Follow;
    if (world.camera.viewW > 0 && world.camera.viewH > 0) {
      const auto cam = game::computeCameraFollow(
          character->x, character->y, world.camera.viewW, world.camera.viewH);
      WorldSetCamera(cam.camX, cam.camY).execute(state);
    }
  }

public:
  explicit SetActiveCombatCharacter(bmin::String _characterId = bmin::String{})
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
