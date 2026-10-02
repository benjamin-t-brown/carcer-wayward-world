#pragma once

#include "game/map/ActiveMapCharacters.h"
#include "model/Combat.h"
#include "state/AbstractAction.hpp"

namespace state {

namespace actions {

class CharacterSetSpriteIndexOffset : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::CharacterSetSpriteIndexOffset; }
  bmin::String characterId;
  int offset = 0;

  void act() override {
    if (!state) {
      return;
    }
    auto* character = game::findCharacterById(state->world.activeMap, characterId);
    if (character == nullptr) {
      return;
    }
    character->spriteIndexOffset = offset;
  }

public:
  CharacterSetSpriteIndexOffset(bmin::String _characterId, int _offset)
      : characterId(std::move(_characterId)), offset(_offset) {}
};

} // namespace actions

} // namespace state
