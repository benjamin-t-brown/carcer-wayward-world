#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileTriggers.h"
#include "model/instances/World.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldInteractAt : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    const auto* avatar =
        game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
    if (!avatar || world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* map = orch.getMapInstanceAt(avatar->x, avatar->y);
    const auto local = orch.activeMapCoordToInstanceCoord(avatar->x, avatar->y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    game::queueActionTravelAtStanding(state->triggers, *map, local.x, local.y);
  }
};

} // namespace actions

} // namespace state
