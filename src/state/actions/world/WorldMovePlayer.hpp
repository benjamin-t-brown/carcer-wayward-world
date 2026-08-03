#pragma once

#include "model/Combat.h"
#include "game/map/MapVision.h"
#include "game/map/MapWalkability.h"
#include "game/map/MapPersistence.h"
#include "game/map/TileTriggers.h"
#include "model/instances/Player.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/String.h"

namespace state {

namespace actions {

// Moves the current party avatar by (dx, dy) tiles, or opens a closed door on bump.
class WorldMovePlayer : public AbstractAction {
  int dx = 0;
  int dy = 0;

  const char* moveDirectionLabel(int dx, int dy) {
    if (dx < 0 && dy < 0) {
      return "nw";
    }
    if (dx == 0 && dy < 0) {
      return "n";
    }
    if (dx > 0 && dy < 0) {
      return "ne";
    }
    if (dx < 0 && dy == 0) {
      return "w";
    }
    if (dx > 0 && dy == 0) {
      return "e";
    }
    if (dx < 0 && dy > 0) {
      return "sw";
    }
    if (dx == 0 && dy > 0) {
      return "s";
    }
    if (dx > 0 && dy > 0) {
      return "se";
    }
    return "?";
  }

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldMovePlayer::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldMovePlayer::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& map = state->world.currentMap;
    if (map.width <= 0 || map.height <= 0) {
      return;
    }

    auto& player = state->player;
    if (player.party.empty()) {
      LOG(ERROR) << "WorldMovePlayer::act: party is empty" << LOG_ENDL;
      return;
    }

    auto partyIndex = player.currentPartyMemberIndex;
    if (partyIndex < 0 ||
        static_cast<size_t>(partyIndex) >= player.party.size()) {
      partyIndex = 0;
    }
    const auto& member = player.party[static_cast<size_t>(partyIndex)];

    model::CharacterInstance* avatar = nullptr;
    for (size_t i = 0; i < map.characters.size(); i++) {
      if (map.characters[i].id == member.instanceId) {
        avatar = &map.characters[i];
        break;
      }
    }
    if (!avatar) {
      LOG(ERROR) << "WorldMovePlayer::act: party avatar not found on map" << LOG_ENDL;
      return;
    }

    model::updateCharacterFacingFromMove(*avatar, dx, dy);

    const auto destX = avatar->x + dx;
    const auto destY = avatar->y + dy;
    LOG(DEBUG) << "WorldMovePlayer: move " << moveDirectionLabel(dx, dy) << LOG_ENDL;

    // 1. Bounds
    if (destX < 0 || destY < 0 || destX >= map.width || destY >= map.height) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    // 2. Closed door (tileset isDoor && !isWalkable) → open, no move
    if (auto* door = game::findClosedDoorAt(map, destX, destY, *database)) {
      door->tileId = door->tileId + 1;
      // TODO(player-tile-movement): play door-open sound
      game::updateMapVisibilityFromPlayer(map, avatar->x, avatar->y, *database);
      return;
    }

    // 3. Effectively walkable (overrides win when authored) → move
    // 4. Else blocked (open doors with walkable meta hit step 3, never step 2)
    if (!game::isDestinationWalkable(map, destX, destY, *database)) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    avatar->x = destX;
    avatar->y = destY;
    game::queueStepTriggersAt(state->triggers, map, destX, destY);
    game::updateMapVisibilityFromPlayer(map, destX, destY, *database);
    if (!state->world.combat.active) {
      game::advanceWorldMovementTicks(state->world, state->mapsByTemplate, 1, *database);
    }
  }

public:
  WorldMovePlayer(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
