#pragma once

#include "model/MapVision.h"
#include "model/TileTriggers.h"
#include "model/instances/Player.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/String.h"

namespace state {

namespace actions {

// Places the current party avatar at tile coordinates on the current map.
// Creates the avatar from the party if it is not already on the map (e.g. after
// WorldLoadMap).
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

    auto& map = state->world.currentMap;
    if (map.width <= 0 || map.height <= 0) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: invalid map dimensions" << LOG_ENDL;
      return;
    }
    if (destX < 0 || destY < 0 || destX >= map.width || destY >= map.height) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: destination out of bounds" << LOG_ENDL;
      return;
    }

    if (!model::placePartyAvatarAt(map, state->player, destX, destY)) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: party is empty" << LOG_ENDL;
      return;
    }

    model::updateMapVisibilityFromPlayer(map, destX, destY, *database);
  }

public:
  WorldSpawnPlayerAtXY(int _destX, int _destY) : destX(_destX), destY(_destY) {}
};

} // namespace actions

} // namespace state
