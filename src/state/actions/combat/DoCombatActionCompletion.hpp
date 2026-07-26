#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/GoNextCombatTurn.hpp"
#include "state/actions/combat/PerformCharacterDefeated.hpp"
#include "state/actions/combat/SetActiveCombatCharacter.hpp"

namespace state {

namespace actions {

class DoCombatActionCompletion : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    auto& combat = world.combat;

    bmin::DynArray<bmin::String> defeatedIds;
    for (const auto& character : world.currentMap.characters) {
      if (model::isCharacterDefeated(state->player, character, *database)) {
        defeatedIds.pushBack(character.id);
      }
    }
    for (const auto& id : defeatedIds) {
      insertCombatAction(new PerformCharacterDefeated(id), 0);
    }

    auto* activeCharacter =
        model::findCharacterOnMap(world.currentMap, combat.activeCharacterId);
    const auto apRemaining = activeCharacter != nullptr ? activeCharacter->currentAp : 0;
    const auto turnEnded = apRemaining <= 0;

    if (turnEnded) {
      insertCombatAction(new GoNextCombatTurn(), 0);
    } else if (activeCharacter != nullptr) {
      insertCombatAction(new SetActiveCombatCharacter(combat.activeCharacterId), 0);
    }
  }

public:
  DoCombatActionCompletion() = default;
};

} // namespace actions

} // namespace state
