#pragma once

#include "bmin/String.h"
#include "game/map/MapPersistence.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class WorldSpawnPlayer : public AbstractAction {
  bmin::String mapName;
  bmin::String markerName;
  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldLoadMap::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldLoadMap::act: state is nullptr" << LOG_ENDL;
      return;
    }

    game::enterMap(state->world, state->mapsByTemplate, mapName, *database);
  }

public:
  explicit WorldSpawnPlayer(const bmin::String& _mapName, const bmin::String& _markerName)
      : mapName(_mapName), markerName(_markerName) {}
};

} // namespace actions

} // namespace state
