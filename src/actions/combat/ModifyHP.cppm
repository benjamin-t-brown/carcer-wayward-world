module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:ModifyHP;
export import carcer.state;
import carcer.game.map;
import carcer.model.templates;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

class ModifyHP : public AbstractAction {
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch;
    auto* character = orch.findCharacterById(characterId);
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

} // export
