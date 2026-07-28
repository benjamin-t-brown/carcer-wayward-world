#pragma once

#include "game/map/MapPersistence.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/String.h"

namespace state {

namespace actions {

class WorldLoadMap : public AbstractAction {
  bmin::String mapName;

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
  explicit WorldLoadMap(bmin::String _mapName) : mapName(std::move(_mapName)) {}
};

} // namespace actions

} // namespace state
