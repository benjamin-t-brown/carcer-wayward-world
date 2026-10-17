#pragma once

#include "actions/general/PlaySound.hpp"
#include "db/Database.h"
#include "game/map/ActiveMapCharacters.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapVision.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileTriggers.h"
#include "model/instances/World.hpp"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"
#include <cstdlib>

namespace state {

namespace actions {

// Confirm Interact/Use at an absolute map tile: requiresUse trigger, close open
// door, or standing requiresAction travel (Chebyshev adjacency ≤ 1).
class WorldInteractAt : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldInteractAt; }
  int x = 0;
  int y = 0;

  static bool isAdjacentOrSame(int ax, int ay, int bx, int by) {
    return std::abs(ax - bx) <= 1 && std::abs(ay - by) <= 1;
  }

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldInteractAt::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldInteractAt::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    const auto* avatar =
        game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
    if (!avatar) {
      return;
    }

    if (!isAdjacentOrSame(avatar->x, avatar->y, x, y)) {
      world.actionMode = model::WorldActionMode::NONE;
      world.actionAimTile.reset();
      LOG(INFO) << TRANSLATE("You need to get closer.") << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances,
                                     getDatabase());
    auto* map = orch.getMapInstanceAt(x, y);
    const auto local = orch.activeMapCoordToInstanceCoord(x, y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      LOG(INFO) << TRANSLATE("You can't see there.") << LOG_ENDL;
      return;
    }

    world.actionMode = model::WorldActionMode::NONE;
    world.actionAimTile.reset();

    const auto* tile = game::tileAtCurrentLayer(*map, local.x, local.y);
    if (tile && tile->eventTrigger && tile->eventTrigger->requiresUse &&
        game::tileEventShouldRun(*tile->eventTrigger, state->specialEventStorage)) {
      state->triggers.pendingSpecialEventId = tile->eventTrigger->eventId;
      if (!tile->eventTrigger->activationSound.empty()) {
        PlaySound(tile->eventTrigger->activationSound).execute(state);
      }
      return;
    }

    if (auto* door = game::findOpenDoorAt(*map, local.x, local.y, *database)) {
      if (game::findCharacterAt(world.activeMap, x, y) == nullptr) {
        game::closeOpenDoor(*door);
        game::persistOpenDoorClose(map->persistentState, map->tileLayerNumber, door->x,
                                   door->y);
        game::updateActiveMapVisibilityFromPlayer(
            world, state->mapInstances, avatar->x, avatar->y, *database);
        return;
      }
    }

    if (x == avatar->x && y == avatar->y) {
      state->triggers.pendingTravel =
          game::resolveActionTravelAtStanding(*map, local.x, local.y);
      return;
    }
  }

public:
  WorldInteractAt(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
