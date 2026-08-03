#pragma once

#include "db/Database.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileTriggers.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldExamineAt : public AbstractAction {
  int x = 0;
  int y = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldExamineAt::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldExamineAt::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* map = orch.getMapInstanceAt(x, y);
    const auto local = orch.activeMapCoordToInstanceCoord(x, y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      LOG(INFO) << "You can't see there." << LOG_ENDL;
      return;
    }

    world.actionMode = model::WorldActionMode::NONE;
    world.actionAimTile.reset();

    const auto* tile = game::tileAtCurrentLayer(*map, local.x, local.y);
    if (tile && tile->eventTrigger && tile->eventTrigger->requiresLook) {
      state->triggers.pendingSpecialEventId = tile->eventTrigger->eventId;
      return;
    }

    LOG(INFO) << game::formatExamineMessage(
                     *map, world.activeMap, x, y, local.x, local.y, *database)
              << LOG_ENDL;
  }

public:
  WorldExamineAt(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
