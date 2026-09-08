#pragma once

#include "model/Combat.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileFields.h"
#include "state/AbstractAction.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/combat/RemoveCharacterFromMap.hpp"

namespace state {

namespace actions {

class PerformCharacterDefeated : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformCharacterDefeated; }
  bmin::String characterId;

  void act() override {
    if (state) {
      game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, getDatabase());
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
    insertAction(state::makeAction<PlaySound>("yell1"), 0);
    insertAction(state::makeAction<RemoveCharacterFromMap>(characterId), 300);
  }

public:
  explicit PerformCharacterDefeated(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
