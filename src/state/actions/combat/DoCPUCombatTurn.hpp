#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/DoCombatAction.hpp"

namespace state {

namespace actions {

class DoCPUCombatTurn : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    insertCombatAction(new DoCombatAction(model::CombatActionType::WAIT), 0);
    // insertCombatAction(new DoCombatActionCompletion(), 0);
  }

public:
  DoCPUCombatTurn() = default;
};

} // namespace actions

} // namespace state
