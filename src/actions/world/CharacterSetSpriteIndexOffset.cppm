module;
#include <cstddef>
#include <utility>

export module carcer.actions.world:CharacterSetSpriteIndexOffset;
export import carcer.state;
import carcer.game.map;

export {

namespace state::actions {

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

} // namespace state::actions

} // export
