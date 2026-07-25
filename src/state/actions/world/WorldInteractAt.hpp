#pragma once

#include "model/TileTriggers.h"
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
        model::findPartyAvatarOnMap(state->world.currentMap, state->player);
    if (!avatar) {
      return;
    }

    model::queueActionTravelAtStanding(
        state->world, state->world.currentMap, avatar->x, avatar->y);
  }
};

} // namespace actions

} // namespace state
