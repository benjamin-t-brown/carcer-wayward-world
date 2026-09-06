module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:CharacterSetSpriteIndexOffset;
export import carcer.state;
import carcer.game.map;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

class CharacterSetSpriteIndexOffset : public AbstractAction {
  bmin::String characterId;
  int offset = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
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

} // export
