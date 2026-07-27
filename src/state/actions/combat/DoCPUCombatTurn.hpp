#pragma once

#include "model/Combat.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/DoCombatAction.hpp"

namespace state {

namespace actions {

class DoCPUCombatTurn : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    LOG(INFO) << "DoCPUCombatTurn: choosing action for "
              << model::formatCharacterLogLabel(state->world.currentMap,
                                                state->world.combat.activeCharacterId)
              << LOG_ENDL;
    insertCombatAction(nullptr, 300);
    insertCombatAction(new DoCombatAction(model::CombatActionType::WAIT), 0);
  }

public:
  DoCPUCombatTurn() = default;
};

} // namespace actions

} // namespace state
