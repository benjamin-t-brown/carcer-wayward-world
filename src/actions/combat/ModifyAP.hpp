#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "model/Combat.h"
#include "actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class ModifyAP : public CombatAction {
  ActionEvent getEvent() const override { return ActionEvent::ModifyAP; }
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, getDatabase());
    auto* character = orch.findCharacterById(characterId);
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
