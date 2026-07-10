#pragma once

#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"

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

    const auto& mapTemplate = database->getMapTemplate(bmin::toStringView(mapName));
    state->world.currentMap = model::createMapInstanceFromTemplate(mapTemplate);
    state->world.name =
        mapTemplate.label.empty() ? mapTemplate.name : mapTemplate.label;
    state->world.camX = 0;
    state->world.camY = 0;
    state->world.cameraMode = model::CameraMode::Follow;
    state->world.cameraFollowCharacterId = bmin::String{};
  }

public:
  explicit WorldLoadMap(bmin::String _mapName) : mapName(std::move(_mapName)) {}
};

} // namespace actions

} // namespace state
