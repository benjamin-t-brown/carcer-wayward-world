module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:RemoveCharacterFromMap;
export import carcer.state;
import carcer.game.map;
import carcer.game.combat;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

class RemoveCharacterFromMap : public AbstractAction {
  bmin::String characterId;

  void act() override {
    if (!state) {
      return;
    }
    auto& characters = state->world.activeMap.characters;
    for (size_t i = 0; i < characters.size();) {
      if (characters[i].id == characterId) {
        if (model::isCharacterEnemy(characters[i])) {
          if (auto* database = getDatabase()) {
            game::markMapCharacterDefeated(
                state->world.activeMap,
                state->mapInstances,
                characters[i],
                *database);
          }
        }
        characters.erase(i);
        if (state->world.combat.active) {
          model::removeCharacterFromCombatTurnOrder(state->world.combat, characterId);
        }
        return;
      }
      i++;
    }
  }

public:
  explicit RemoveCharacterFromMap(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state

} // export
