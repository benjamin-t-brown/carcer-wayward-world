module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:EndCombat;
export import carcer.state;
import carcer.game.map;
import carcer.data;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class EndCombat : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    LOG(INFO) << "EndCombat: ending combat, returning to town mode" << LOG_ENDL;
    world.combat.active = false;
    world.combat.turnOrderIds.clear();
    world.combat.activeTurnIndex = 0;
    world.combat.activeCharacterId = bmin::String{};
    world.combat.isWaitingForAction = false;
    state->turnMode = model::TurnMode::TURN_TOWN;

    model::removeExtraPartyMembersFromMap(world, state->player);

    if (state->player.party.empty()) {
      return;
    }

    const auto& leader = state->player.party[0];
    world.camera.cameraFollowCharacterId = leader.instanceId;
    world.camera.cameraMode = model::CameraMode::Follow;

    if (auto* avatar =
            game::findPartyAvatarOnActiveMap(world.activeMap, state->player)) {
      auto* database = getDatabase();
      if (database != nullptr) {
        game::updateActiveMapVisibilityFromPlayer(
            world, state->mapInstances, avatar->x, avatar->y, *database);
      }
      if (world.camera.viewW > 0 && world.camera.viewH > 0) {
        const auto cam = game::computeCameraFollow(
            avatar->x, avatar->y, world.camera.viewW, world.camera.viewH);
        world.camera.camX = cam.camX;
        world.camera.camY = cam.camY;
      }
    }
  }

public:
  EndCombat() = default;
};

} // namespace actions

} // namespace state

} // export
