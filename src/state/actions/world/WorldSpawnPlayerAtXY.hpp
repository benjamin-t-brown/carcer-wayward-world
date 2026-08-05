#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapVision.h"
#include "game/map/TileTriggers.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/String.h"

namespace state {

namespace actions {

// Places the current party avatar at world tile coordinates on the active map.
class WorldSpawnPlayerAtXY : public AbstractAction {
  int destX = 0;
  int destY = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: no active map loaded" << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: destination out of bounds" << LOG_ENDL;
      return;
    }

    if (!game::placePartyAvatarAt(
            world.activeMap, state->player, destX, destY, database)) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: party is empty" << LOG_ENDL;
      return;
    }

    game::updateActiveMapVisibilityFromPlayer(world, destX, destY, *database);
  }

public:
  WorldSpawnPlayerAtXY(int _destX, int _destY) : destX(_destX), destY(_destY) {}
};

} // namespace actions

} // namespace state
