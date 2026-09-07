#pragma once

#include "state/AbstractAction.h"
#include "state/State.h"

namespace state::actions {

class ClearTownEnemyAiResolving : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::ClearTownEnemyAiResolving;
  }

  void act() override {
    if (state) {
      state->world.resolvingTownEnemyAi = false;
    }
  }
};

} // namespace state::actions
