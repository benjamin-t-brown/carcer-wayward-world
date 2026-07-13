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

class WorldExamineDirection : public AbstractAction {
  int dx = 0;
  int dy = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldExamineDirection::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldExamineDirection::act: state is nullptr" << LOG_ENDL;
      return;
    }

    state->world.actionMode = model::WorldActionMode::NONE;

    auto& map = state->world.currentMap;
    const auto* avatar = model::findPartyAvatarOnMap(map, state->player);
    if (!avatar) {
      LOG(ERROR) << "WorldExamineDirection::act: party avatar not found on map" << LOG_ENDL;
      return;
    }

    const auto targetX = avatar->x + dx;
    const auto targetY = avatar->y + dy;
    if (targetX < 0 || targetY < 0 || targetX >= map.width || targetY >= map.height) {
      return;
    }

    const auto* tile = model::tileAtCurrentLayer(map, targetX, targetY);
    if (tile && tile->eventTrigger && tile->eventTrigger->isLookTrigger) {
      state->world.pendingSpecialEventId = tile->eventTrigger->eventId;
      return;
    }

    LOG(INFO) << model::formatExamineMessage(map, targetX, targetY, *database) << LOG_ENDL;
  }

public:
  WorldExamineDirection(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
