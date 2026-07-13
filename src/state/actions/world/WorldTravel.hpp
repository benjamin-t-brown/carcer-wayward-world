#pragma once

#include "model/templates/Maps.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "state/actions/world/WorldLoadMap.hpp"
#include "state/actions/world/WorldSpawnPlayerAtMarker.hpp"
#include "state/actions/world/WorldSpawnPlayerAtXY.hpp"
#include "bmin/String.h"

namespace state {

namespace actions {

class WorldTravel : public AbstractAction {
  model::TravelTrigger travel;

  void act() override {
    if (!state) {
      return;
    }
    if (travel.destinationMapName.empty()) {
      return;
    }

    WorldLoadMap(travel.destinationMapName).execute(state);

    if (!travel.destinationMarkerName.empty()) {
      WorldSpawnPlayerAtMarker(travel.destinationMarkerName).execute(state);
    } else {
      WorldSpawnPlayerAtXY(travel.destinationX, travel.destinationY).execute(state);
    }
  }

public:
  explicit WorldTravel(model::TravelTrigger _travel) : travel(std::move(_travel)) {}
};

} // namespace actions

} // namespace state
