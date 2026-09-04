module;
#include <utility>

export module carcer.actions.combat.CharacterSetSpriteIndexOffset;
export import carcer.actions.combat.CombatAction;
import carcer.game.map;

export {

namespace state {

namespace actions {

class CharacterSetSpriteIndexOffset : public CombatAction {
  bmin::String characterId;
  int offset = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch;
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
