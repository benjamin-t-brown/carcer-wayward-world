#pragma once

#include "model/instances/World.h"
#include "model/templates/Maps.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "state/actions/world/WorldLoadMap.hpp"
#include "state/actions/world/WorldSpawnPlayerAtMarker.hpp"
#include "state/actions/world/WorldSpawnPlayerAtXY.hpp"
#include "bmin/String.h"
#include "bmin/StringInterop.h"

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

    auto usedMarker = false;
    if (!travel.destinationMarkerName.empty()) {
      auto* database = getDatabase();
      if (database) {
        auto& map = state->world.currentMap;
        if (!map.templateName.empty()) {
          const auto& mapTemplate =
              database->getMapTemplate(bmin::toStringView(map.templateName));
          if (model::findMarkerOnTemplate(mapTemplate, travel.destinationMarkerName)) {
            WorldSpawnPlayerAtMarker(travel.destinationMarkerName).execute(state);
            usedMarker = true;
          } else {
            LOG(WARN) << "WorldTravel::act: marker not found on destination map, "
                         "falling back to XY: "
                      << travel.destinationMarkerName << LOG_ENDL;
          }
        }
      }
    }

    if (!usedMarker) {
      state->world.currentMap.tileLayerNumber = travel.destinationLayer;
      WorldSpawnPlayerAtXY(travel.destinationX, travel.destinationY).execute(state);
    }
  }

public:
  explicit WorldTravel(model::TravelTrigger _travel) : travel(std::move(_travel)) {}
};

} // namespace actions

} // namespace state
