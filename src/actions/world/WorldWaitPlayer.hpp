#pragma once

#include "actions/world/TownEnemyAiAfterPlayerMove.hpp"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapPersistence.h"
#include "game/map/TileTriggers.h"
#include "model/instances/World.hpp"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

// Town wait (keypad 5 / wait button): standing requires-action travel trigger,
// then one overworld turn so town enemy AI may act (same as a player move).
class WorldWaitPlayer : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldWaitPlayer; }
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    if (world.combat.active) {
      return;
    }

    const auto* avatar =
        game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
    if (avatar != nullptr && !world.activeMap.gridId.empty()) {
      game::ActiveMapOrchestrator orch(
          state->world.activeMap, state->mapInstances, getDatabase());
      auto* map = orch.getMapInstanceAt(avatar->x, avatar->y);
      const auto local = orch.activeMapCoordToInstanceCoord(avatar->x, avatar->y);
      if (map != nullptr && local.valid) {
        map->tileLayerNumber = world.activeMap.mapLayer;
        state->triggers.pendingTravel =
            game::resolveActionTravelAtStanding(*map, local.x, local.y);
      }
    }

    state->playerMovementCount += 1;
    game::ageMapInstances(state->mapInstances, 1);
    world.resolvingTownEnemyAi = true;
    insertAction(state::makeAction<TownEnemyAiAfterPlayerMove>(), 0);
  }

public:
  WorldWaitPlayer() = default;
};

} // namespace actions

} // namespace state
