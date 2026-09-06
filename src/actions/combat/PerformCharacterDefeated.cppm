module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:PerformCharacterDefeated;
export import carcer.state;
import :RemoveCharacterFromMap;
import carcer.actions.general;
import carcer.game.map;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

class PerformCharacterDefeated : public AbstractAction {
  bmin::String characterId;

  void act() override {
    if (state) {
      game::ActiveMapOrchestrator orch(
          state->world.activeMap, state->mapInstances, getDatabase());
      if (!state->world.activeMap.gridId.empty()) {
        orch.fetchMapGrid(state->world.activeMap.gridId);
      }
      if (auto* character = orch.findCharacterById(characterId)) {
        auto* map = orch.getMapInstanceAt(character->x, character->y);
        const auto local =
            orch.activeMapCoordToInstanceCoord(character->x, character->y);
        if (map && local.valid) {
          map->tileLayerNumber = state->world.activeMap.mapLayer;
          game::addTileFieldAt(*map, local.x, local.y, game::TileFieldType::BLOOD);
        }
      }
    }
    insertAction(new PlaySound("yell1"), 0);
    insertAction(new RemoveCharacterFromMap(characterId), 300);
  }

public:
  explicit PerformCharacterDefeated(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state

} // export
