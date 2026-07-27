#pragma once

#include "model/Combat.h"
#include "sdl2w/Logger.h"
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

    LOG(INFO) << "GoNextCombatTurn: advancing from turn index " << combat.activeTurnIndex
              << LOG_ENDL;

    combat.activeTurnIndex += 1;
    if (combat.activeTurnIndex >= static_cast<int>(combat.turnOrderIds.size())) {
      LOG(INFO) << "GoNextCombatTurn: new combat round, resetting AP" << LOG_ENDL;
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
      LOG(INFO) << "GoNextCombatTurn: next actor is "
                << model::formatCharacterLogLabel(state->world.currentMap, nextId) << LOG_ENDL;
      return;
    }
  }

public:
  GoNextCombatTurn() = default;
};

} // namespace actions

} // namespace state
