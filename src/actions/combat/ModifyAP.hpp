#pragma once

#include "game/map/ActiveMapCharacters.h"
#include "model/Combat.h"
#include "state/AbstractAction.hpp"

namespace state {

namespace actions {

class ModifyAP : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::ModifyAP; }
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    auto* character = game::findCharacterById(state->world.activeMap, characterId);
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
