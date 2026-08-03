#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/Combat.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapVision.h"
#include "game/map/MapWalkability.h"
#include "state/actions/combat/ActionBase.hpp"

namespace state {

namespace actions {

class MoveCharacter : public CombatAction {
  bmin::String characterId;
  int dx = 0;
  int dy = 0;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }

    const auto destX = character->x + dx;
    const auto destY = character->y + dy;
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      return;
    }
    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      return;
    }
    if (orch.findCharacterAt(destX, destY, characterId) != nullptr) {
      return;
    }

    character->x = destX;
    character->y = destY;
    model::updateCharacterFacingFromMove(*character, dx, dy);

    if (model::isPartyMember(state->player, character->id)) {
      game::updateActiveMapVisibilityFromParty(world, state->player, *database);
    }
  }

public:
  MoveCharacter(bmin::String _characterId, int _dx, int _dy)
      : characterId(std::move(_characterId)), dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
