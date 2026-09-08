#pragma once

#include "state/AbstractAction.hpp"
#include "state/State.hpp"

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
