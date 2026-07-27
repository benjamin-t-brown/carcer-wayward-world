#pragma once

#include "model/Combat.h"
#include "model/MapPersistence.h"
#include "state/actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class RemoveCharacterFromMap : public CombatAction {
  bmin::String characterId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    auto& characters = state->world.currentMap.characters;
    for (size_t i = 0; i < characters.size();) {
      if (characters[i].id == characterId) {
        if (database != nullptr && model::isCharacterEnemy(characters[i], *database)) {
          model::markMapCharacterDefeated(state->world, characters[i]);
        }
        characters.erase(i);
        if (state->world.combat.active) {
          model::removeCharacterFromCombatTurnOrder(state->world.combat, characterId);
        }
        return;
      }
      i++;
    }
  }

public:
  explicit RemoveCharacterFromMap(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
