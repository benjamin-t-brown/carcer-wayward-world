module;
#include <utility>

export module carcer.actions.combat.ModifyAP;
export import carcer.actions.combat.CombatAction;
import carcer.game.map;

export {

namespace state {

namespace actions {

class ModifyAP : public CombatAction {
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
    character->currentAp += delta;
  }

public:
  ModifyAP(bmin::String _characterId, int _delta)
      : characterId(std::move(_characterId)), delta(_delta) {}
};

} // namespace actions

} // namespace state

} // export
