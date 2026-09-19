#pragma once

#include "actions/combat/RemoveCharacterFromMap.hpp"
#include "actions/general/PlaySound.hpp"
#include "bmin/StringInterop.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileFields.h"
#include "model/Combat.h"
#include "state/AbstractAction.hpp"

namespace state {

namespace actions {

class PerformCharacterDefeated : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformCharacterDefeated; }
  bmin::String characterId;

  void act() override {
    bmin::String deathSoundName = "yell_monster1";
    if (state) {
      auto* database = getDatabase();
      game::ActiveMapOrchestrator orch(
          state->world.activeMap, state->mapInstances, database);
      if (!state->world.activeMap.gridId.empty()) {
        orch.fetchMapGrid(state->world.activeMap.gridId);
      }
      if (auto* character = orch.findCharacterById(characterId)) {
        auto* map = orch.getMapInstanceAt(character->x, character->y);
        const auto local = orch.activeMapCoordToInstanceCoord(character->x, character->y);
        if (map && local.valid) {
          map->tileLayerNumber = state->world.activeMap.mapLayer;
          game::addTileFieldAt(*map, local.x, local.y, game::TileFieldType::BLOOD);
        }
        if (database && !character->templateName.empty()) {
          try {
            const auto& characterTemplate = database->getCharacterTemplate(
                bmin::toStringView(character->templateName));
            deathSoundName = characterTemplate.sound.deathSoundName;
          } catch (...) {
          }
        }
      }
    }

    insertAction(state::makeAction<PlaySound>(deathSoundName), 0);
    insertAction(state::makeAction<RemoveCharacterFromMap>(characterId), 300);
  }

public:
  explicit PerformCharacterDefeated(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
