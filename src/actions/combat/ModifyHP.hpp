#pragma once

#include "game/map/ActiveMapCharacters.h"
#include "model/Combat.h"
#include "state/AbstractAction.hpp"

namespace state {

namespace actions {

class ModifyHP : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::ModifyHP; }
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
    const auto hp = model::getCharacterHp(state->player, *character) + delta;
    model::setCharacterHp(state->player, *character, hp);
  }

public:
  ModifyHP(bmin::String _characterId, int _delta)
      : characterId(std::move(_characterId)), delta(_delta) {}
};

} // namespace actions

} // namespace state
