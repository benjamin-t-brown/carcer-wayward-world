#pragma once

#include "model/MapPersistence.h"
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

    model::enterMap(state->world, mapName, *database);
  }

public:
  explicit WorldLoadMap(bmin::String _mapName) : mapName(std::move(_mapName)) {}
};

} // namespace actions

} // namespace state
