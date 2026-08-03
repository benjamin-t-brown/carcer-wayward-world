#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class ModifyHP : public CombatAction {
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }
    auto* character = model::mapInstanceFindCharacter(state->world.currentMap, characterId);
    if (character == nullptr) {
      return;
    }
    const auto hp =
        model::getCharacterHp(state->player, *character, *database) + delta;
    model::setCharacterHp(state->player, *character, hp, *database);
  }

public:
  ModifyHP(bmin::String _characterId, int _delta)
      : characterId(std::move(_characterId)), delta(_delta) {}
};

} // namespace actions

} // namespace state
