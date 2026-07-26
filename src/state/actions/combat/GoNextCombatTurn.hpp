#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/SetActiveCombatCharacter.hpp"

namespace state {

namespace actions {

class GoNextCombatTurn : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& combat = state->world.combat;
    if (!combat.active || combat.turnOrderIds.empty()) {
      return;
    }

    combat.activeTurnIndex += 1;
    if (combat.activeTurnIndex >= static_cast<int>(combat.turnOrderIds.size())) {
      combat.activeTurnIndex = 0;
      model::resetAllCombatAp(state->world, model::COMBAT_STARTING_AP);
    }

    const auto turnCount = static_cast<int>(combat.turnOrderIds.size());
    for (int attempt = 0; attempt < turnCount; attempt++) {
      const auto index = combat.activeTurnIndex;
      if (index < 0 || index >= turnCount) {
        break;
      }
      const auto& nextId = combat.turnOrderIds[static_cast<size_t>(index)];
      auto* nextCharacter = model::findCharacterOnMap(state->world.currentMap, nextId);
      if (nextCharacter == nullptr) {
        combat.activeTurnIndex += 1;
        if (combat.activeTurnIndex >= turnCount) {
          combat.activeTurnIndex = 0;
          model::resetAllCombatAp(state->world, model::COMBAT_STARTING_AP);
        }
        continue;
      }
      if (model::isCharacterDefeated(state->player, *nextCharacter, *database)) {
        combat.activeTurnIndex += 1;
        if (combat.activeTurnIndex >= turnCount) {
          combat.activeTurnIndex = 0;
          model::resetAllCombatAp(state->world, model::COMBAT_STARTING_AP);
        }
        continue;
      }
      insertCombatAction(new SetActiveCombatCharacter(nextId), 0);
      return;
    }
  }

public:
  GoNextCombatTurn() = default;
};

} // namespace actions

} // namespace state
