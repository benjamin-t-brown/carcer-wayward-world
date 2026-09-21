#pragma once

#include "bmin/StringInterop.h"
#include "game/combat/StatusRules.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "actions/combat/PerformStatusAbility.hpp"
#include "actions/combat/SetActiveCombatCharacter.hpp"

namespace state {

namespace actions {

class TickCombatStatuses : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::TickCombatStatuses; }
  bmin::String characterId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }
    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, database);
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }

    if (!character->statusEffects.empty()) {
      bmin::DynArray<bmin::String> abilityNames;
      game::collectTurnStartStatusAbilities(*character, *database, abilityNames);
      LOG(DEBUG) << "TickCombatStatuses: " << characterId << " actions="
                 << static_cast<int>(abilityNames.size()) << LOG_ENDL;
      for (size_t i = 0; i < abilityNames.size(); i++) {
        insertAction(state::makeAction<PerformStatusAbility>(characterId, abilityNames[i]),
                     static_cast<int>(i) * 50);
      }
    }
    // Status ticks (and their depictions) finish before the unit can act.
    insertAction(state::makeAction<SetActiveCombatCharacter>(characterId), 0);
  }

public:
  explicit TickCombatStatuses(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
