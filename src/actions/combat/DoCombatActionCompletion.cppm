export module carcer.actions.combat.DoCombatActionCompletion;
export import carcer.actions.combat.CombatAction;
import sdl2w;
import carcer.actions.combat.GoNextCombatTurn;
import carcer.actions.combat.PerformCharacterDefeated;
import carcer.actions.combat.SetActiveCombatCharacter;
import carcer.game.map;
#include "macros.h"

export {

namespace state {

namespace actions {

class DoCombatActionCompletion : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    auto& combat = world.combat;

    LOG(INFO) << "DoCombatActionCompletion: checking results for "
              << model::formatCharacterLogLabel(world.activeMap, combat.activeCharacterId)
              << LOG_ENDL;

    bmin::DynArray<bmin::String> defeatedIds;
    for (const auto& character : world.activeMap.characters) {
      if (model::isCharacterDefeated(state->player, character)) {
        defeatedIds.pushBack(character.id);
      }
    }
    for (const auto& id : defeatedIds) {
      insertAction(new PerformCharacterDefeated(id), 0);
    }

    game::ActiveMapOrchestrator orch;
    auto* activeCharacter = orch.findCharacterById(combat.activeCharacterId);
    const auto apRemaining = activeCharacter != nullptr ? activeCharacter->currentAp : 0;
    const auto turnEnded = apRemaining <= 0;

    if (turnEnded) {
      LOG(INFO) << "DoCombatActionCompletion: turn ended, advancing to next character"
                << LOG_ENDL;
      insertAction(new GoNextCombatTurn(), 0);
    } else if (activeCharacter != nullptr) {
      LOG(INFO) << "DoCombatActionCompletion: "
                << model::formatCharacterLogLabel(world.activeMap, combat.activeCharacterId)
                << " has " << apRemaining << " AP remaining, waiting for next action"
                << LOG_ENDL;
      insertAction(new SetActiveCombatCharacter(combat.activeCharacterId), 0);
    }
  }

public:
  DoCombatActionCompletion() = default;
};

} // namespace actions

} // namespace state

} // export
