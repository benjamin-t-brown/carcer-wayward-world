#pragma once

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

    const auto* avatar =
        game::findPartyAvatarOnMap(state->world.currentMap, state->player);
    if (!avatar) {
      return;
    }

    game::queueActionTravelAtStanding(
        state->triggers, state->world.currentMap, avatar->x, avatar->y);
  }
};

} // namespace actions

} // namespace state
