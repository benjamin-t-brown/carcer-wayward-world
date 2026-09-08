#pragma once

#include "game/map/ActiveMapOrchestrator.h"
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
    game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, getDatabase());
    auto* character = orch.findCharacterById(characterId);
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
