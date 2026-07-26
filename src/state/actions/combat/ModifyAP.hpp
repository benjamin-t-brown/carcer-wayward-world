#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class ModifyAP : public CombatAction {
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    auto* character = model::findCharacterOnMap(state->world.currentMap, characterId);
    if (character == nullptr) {
      return;
    }
    character->currentAp += delta;
  }

public:
  ModifyAP(bmin::String _characterId, int _delta)
      : characterId(std::move(_characterId)), delta(_delta) {}
};

} // namespace actions

} // namespace state
