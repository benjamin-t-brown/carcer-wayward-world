#pragma once

#include "db/Database.h"
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

    auto& map = state->world.currentMap;
    if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
      return;
    }

    if (!game::isTileCurrentlyVisible(map, x, y)) {
      LOG(INFO) << "You can't see there." << LOG_ENDL;
      return;
    }

    state->world.actionMode = model::WorldActionMode::NONE;
    state->world.actionAimTile.reset();

    const auto* tile = game::tileAtCurrentLayer(map, x, y);
    if (tile && tile->eventTrigger && tile->eventTrigger->requiresLook) {
      state->triggers.pendingSpecialEventId = tile->eventTrigger->eventId;
      return;
    }

    LOG(INFO) << game::formatExamineMessage(map, x, y, *database) << LOG_ENDL;
  }

public:
  WorldExamineAt(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
