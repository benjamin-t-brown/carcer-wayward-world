#pragma once

#include "db/Database.h"
#include "model/MapWalkability.h"
#include "model/TileTriggers.h"
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

    auto& map = state->world.currentMap;
    if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
      return;
    }

    if (!model::isTileCurrentlyVisible(map, x, y)) {
      LOG(INFO) << "You can't see there." << LOG_ENDL;
      return;
    }

    state->world.actionMode = model::WorldActionMode::NONE;
    state->world.actionAimTile.reset();

    const auto* tile = model::tileAtCurrentLayer(map, x, y);
    if (tile && tile->eventTrigger && tile->eventTrigger->requiresLook) {
      state->world.pendingSpecialEventId = tile->eventTrigger->eventId;
      return;
    }

    LOG(INFO) << model::formatExamineMessage(map, x, y, *database) << LOG_ENDL;
  }

public:
  WorldExamineAt(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
