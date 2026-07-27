#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class CharacterSetSpriteIndexOffset : public CombatAction {
  bmin::String characterId;
  int offset = 0;

  void act() override {
    if (!state) {
      return;
    }
    auto* character = model::findCharacterOnMap(state->world.currentMap, characterId);
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
