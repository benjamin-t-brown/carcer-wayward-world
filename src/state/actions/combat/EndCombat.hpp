#pragma once

#include "model/Combat.h"
#include "game/map/Camera.h"
#include "game/map/MapVision.h"
#include "game/map/TileTriggers.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/world/WorldSetCamera.hpp"

namespace state {

namespace actions {

class EndCombat : public CombatAction {
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
    world.currentMap.turnMode = model::TurnMode::TURN_TOWN;

    model::removeExtraPartyMembersFromMap(world, state->player);

    if (state->player.party.empty()) {
      return;
    }

    const auto& leader = state->player.party[0];
    world.camera.cameraFollowCharacterId = leader.instanceId;
    world.camera.cameraMode = model::CameraMode::Follow;

    if (auto* avatar = game::findPartyAvatarOnMap(world.currentMap, state->player)) {
      auto* database = getDatabase();
      if (database != nullptr) {
        game::updateMapVisibilityFromPlayer(
            world.currentMap, avatar->x, avatar->y, *database);
      }
      if (world.camera.viewW > 0 && world.camera.viewH > 0) {
        const auto cam = game::computeCameraFollow(avatar->x,
                                                    avatar->y,
                                                    world.currentMap,
                                                    world.camera.viewW,
                                                    world.camera.viewH);
        WorldSetCamera(cam.camX, cam.camY).execute(state);
      }
    }
  }

public:
  EndCombat() = default;
};

} // namespace actions

} // namespace state
