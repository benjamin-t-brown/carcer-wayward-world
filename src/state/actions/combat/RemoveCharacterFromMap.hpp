#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class RemoveCharacterFromMap : public CombatAction {
  bmin::String characterId;

  void act() override {
    if (!state) {
      return;
    }
    auto& characters = state->world.currentMap.characters;
    for (size_t i = 0; i < characters.size();) {
      if (characters[i].id == characterId) {
        characters.erase(i);
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
