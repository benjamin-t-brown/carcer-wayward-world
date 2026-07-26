#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/PlaySound.hpp"
#include "state/actions/combat/RemoveCharacterFromMap.hpp"

namespace state {

namespace actions {

class PerformCharacterDefeated : public CombatAction {
  bmin::String characterId;

  void act() override {
    insertCombatAction(new PlaySound("yell1"), 0);
    insertCombatAction(new RemoveCharacterFromMap(characterId), 300);
  }

public:
  explicit PerformCharacterDefeated(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
