#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/Combat.h"
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

    auto& map = state->world.currentMap;
    auto* character = model::mapInstanceFindCharacter(map, characterId);
    if (character == nullptr) {
      return;
    }

    const auto destX = character->x + dx;
    const auto destY = character->y + dy;
    if (destX < 0 || destY < 0 || destX >= map.width || destY >= map.height) {
      return;
    }
    if (!game::isDestinationWalkable(map, destX, destY, *database)) {
      return;
    }
    if (model::findCharacterAt(map, destX, destY, characterId) != nullptr) {
      return;
    }

    character->x = destX;
    character->y = destY;
    model::updateCharacterFacingFromMove(*character, dx, dy);

    if (model::isPartyMember(state->player, character->id)) {
      game::updateMapVisibilityFromParty(map, state->player, *database);
    }
  }

public:
  MoveCharacter(bmin::String _characterId, int _dx, int _dy)
      : characterId(std::move(_characterId)), dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
