#pragma once

#include "model/Combat.h"
#include "game/map/ActiveMapOrchestrator.h"
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

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || total.x <= 0 || total.y <= 0) {
      return;
    }

    auto& player = state->player;
    if (player.party.empty()) {
      LOG(ERROR) << "WorldMovePlayer::act: party is empty" << LOG_ENDL;
      return;
    }

    auto* avatar = game::findPartyAvatarOnActiveMap(world.activeMap, player);
    if (!avatar) {
      LOG(ERROR) << "WorldMovePlayer::act: party avatar not found on map" << LOG_ENDL;
      return;
    }

    model::updateCharacterFacingFromMove(*avatar, dx, dy);

    const auto destX = avatar->x + dx;
    const auto destY = avatar->y + dy;
    LOG(DEBUG) << "WorldMovePlayer: move " << moveDirectionLabel(dx, dy) << LOG_ENDL;

    if (destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;

    if (auto* door = game::findClosedDoorAt(*destMap, destLocal.x, destLocal.y, *database)) {
      door->tileId = door->tileId + 1;
      game::updateActiveMapVisibilityFromPlayer(world, avatar->x, avatar->y, *database);
      return;
    }

    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    avatar->x = destX;
    avatar->y = destY;
    game::queueStepTriggersAt(state->triggers, *destMap, destLocal.x, destLocal.y);
    game::updateActiveMapVisibilityFromPlayer(world, destX, destY, *database);
    if (!world.combat.active) {
      game::advanceWorldMovementTicks(*state, 1);
    }
  }

public:
  WorldMovePlayer(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
