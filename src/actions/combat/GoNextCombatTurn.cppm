module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:GoNextCombatTurn;
export import :CombatAction;
import :SetActiveCombatCharacter;
import carcer.game.map;
import carcer.game.combat;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class GoNextCombatTurn : public CombatAction {
  void startNewCombatRound() {
    LOG(INFO) << "GoNextCombatTurn: new combat round, resetting AP" << LOG_ENDL;
    state->world.combat.activeTurnIndex = 0;
    game::onNewCombatRound(*state);
  }

  void act() override {
    if (!state) {
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
      startNewCombatRound();
    }

    game::ActiveMapOrchestrator orch;
    const auto turnCount = static_cast<int>(combat.turnOrderIds.size());
    for (int attempt = 0; attempt < turnCount; attempt++) {
      const auto index = combat.activeTurnIndex;
      if (index < 0 || index >= turnCount) {
        break;
      }
      const auto& nextId = combat.turnOrderIds[static_cast<size_t>(index)];
      auto* nextCharacter = orch.findCharacterById(nextId);
      if (nextCharacter == nullptr) {
        combat.activeTurnIndex += 1;
        if (combat.activeTurnIndex >= turnCount) {
          startNewCombatRound();
        }
        continue;
      }
      if (model::isCharacterDefeated(state->player, *nextCharacter)) {
        combat.activeTurnIndex += 1;
        if (combat.activeTurnIndex >= turnCount) {
          startNewCombatRound();
        }
        continue;
      }
      insertAction(new SetActiveCombatCharacter(nextId), 0);
      LOG(INFO) << "GoNextCombatTurn: next actor is "
                << model::formatCharacterLogLabel(state->world.activeMap, nextId)
                << LOG_ENDL;
      return;
    }
  }

public:
  GoNextCombatTurn() = default;
};

} // namespace actions

} // namespace state

} // export
