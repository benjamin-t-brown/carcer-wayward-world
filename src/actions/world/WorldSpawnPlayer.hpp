#pragma once

#include "bmin/String.h"
#include "game/map/MapPersistence.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"
#include "actions/world/WorldLoadActiveMap.hpp"
#include "actions/world/WorldSpawnPlayerAtMarker.hpp"

namespace state {

namespace actions {

class WorldSpawnPlayer : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSpawnPlayer; }
  bmin::String mapName;
  bmin::String markerName;
  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayer::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayer::act: state is nullptr" << LOG_ENDL;
      return;
    }

    const auto gridId = game::resolveGridIdForMapOrGrid(*database, mapName);
    if (gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayer::act: could not resolve grid for " << mapName
                 << LOG_ENDL;
      return;
    }

    WorldLoadActiveMap(gridId).execute(state);
    WorldSpawnPlayerAtMarker(markerName).execute(state);
  }

public:
  explicit WorldSpawnPlayer(const bmin::String& _mapName, const bmin::String& _markerName)
      : mapName(_mapName), markerName(_markerName) {}
};

} // namespace actions

} // namespace state
