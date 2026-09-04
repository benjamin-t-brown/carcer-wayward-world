module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:SetActiveCombatCharacter;
export import :CombatAction;
import carcer.game.map;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

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

    if (model::isPartyMember(state->player, characterId)) {
      // Highlight the acting party member in the HUD only.
      state->uiState.selectedPartyMemberId = characterId;
    }

    world.camera.cameraFollowCharacterId = characterId;
    world.camera.cameraMode = model::CameraMode::Follow;
    if (world.camera.viewW > 0 && world.camera.viewH > 0) {
      const auto cam = game::computeCameraFollow(
          character->x, character->y, world.camera.viewW, world.camera.viewH);
      world.camera.camX = cam.camX;
      world.camera.camY = cam.camY;
    }
  }

public:
  explicit SetActiveCombatCharacter(bmin::String _characterId = bmin::String{})
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state

} // export
