#pragma once

#include "model/instances/Player.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/String.h"

namespace state {

namespace actions {

// Places the current party avatar at tile coordinates on the current map.
class WorldSpawnPlayerAtXY : public AbstractAction {
  int destX = 0;
  int destY = 0;

  void act() override {
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

    auto* avatar = model::findPartyAvatarOnMap(map, state->player);
    if (!avatar) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: party avatar not found on map" << LOG_ENDL;
      return;
    }

    avatar->x = destX;
    avatar->y = destY;
  }

public:
  WorldSpawnPlayerAtXY(int _destX, int _destY) : destX(_destX), destY(_destY) {}
};

} // namespace actions

} // namespace state
