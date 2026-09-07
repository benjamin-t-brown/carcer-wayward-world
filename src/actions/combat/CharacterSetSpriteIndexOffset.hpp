#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "model/Combat.h"
#include "actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class CharacterSetSpriteIndexOffset : public CombatAction {
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
