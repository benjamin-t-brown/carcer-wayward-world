#pragma once

#include "model/Combat.h"
#include "sdl2w/Logger.h"
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

    LOG(INFO) << "DoCombatActionCompletion: checking results for "
              << model::formatCharacterLogLabel(world.currentMap, combat.activeCharacterId)
              << LOG_ENDL;

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
        model::mapInstanceFindCharacter(world.currentMap, combat.activeCharacterId);
    const auto apRemaining = activeCharacter != nullptr ? activeCharacter->currentAp : 0;
    const auto turnEnded = apRemaining <= 0;

    if (turnEnded) {
      LOG(INFO) << "DoCombatActionCompletion: turn ended, advancing to next character"
                << LOG_ENDL;
      insertCombatAction(new GoNextCombatTurn(), 0);
    } else if (activeCharacter != nullptr) {
      LOG(INFO) << "DoCombatActionCompletion: "
                << model::formatCharacterLogLabel(world.currentMap, combat.activeCharacterId)
                << " has " << apRemaining << " AP remaining, waiting for next action"
                << LOG_ENDL;
      insertCombatAction(new SetActiveCombatCharacter(combat.activeCharacterId), 0);
    }
  }

public:
  DoCombatActionCompletion() = default;
};

} // namespace actions

} // namespace state
