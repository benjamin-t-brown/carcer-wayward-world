#pragma once

#include "model/instances/World.hpp"
#include "model/templates/Maps.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapPersistence.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"
#include "actions/world/WorldLoadActiveMap.hpp"
#include "actions/world/WorldSpawnPlayerAtMarker.hpp"
#include "actions/world/WorldSpawnPlayerAtXY.hpp"
#include "bmin/String.h"
#include "bmin/StringInterop.h"

namespace state {

namespace actions {

class WorldTravel : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldTravel; }
  model::TravelTrigger travel;

  void act() override {
    if (!state) {
      return;
    }
    if (travel.destinationMapName.empty()) {
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      return;
    }

    const auto gridId =
        game::resolveGridIdForMapOrGrid(*database, travel.destinationMapName);
    if (gridId.empty()) {
      LOG(ERROR) << "WorldTravel::act: could not resolve grid for "
                 << travel.destinationMapName << LOG_ENDL;
      return;
    }

    // Same grid: teleport only. Different grid: unload/reload active map entities.
    if (state->world.activeMap.gridId != gridId) {
      WorldLoadActiveMap(gridId).execute(state);
    }

    auto usedMarker = false;
    if (!travel.destinationMarkerName.empty()) {
      game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, getDatabase());
      orch.fetchMapGrid(gridId);
      const auto marker =
          orch.findMarker(travel.destinationMapName, travel.destinationMarkerName);
      if (marker.valid) {
        state->world.activeMap.mapLayer = marker.layer;
        WorldSpawnPlayerAtXY(marker.x, marker.y).execute(state);
        usedMarker = true;
      } else {
        LOG(WARN) << "WorldTravel::act: marker not found on destination map, "
                     "falling back to XY: "
                  << travel.destinationMarkerName << LOG_ENDL;
      }
    }

    if (!usedMarker) {
      state->world.activeMap.mapLayer = travel.destinationLayer;
      // destinationX/Y are local to destinationMapName — convert to world.
      game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, getDatabase());
      orch.fetchMapGrid(gridId);
      const auto worldLoc = orch.instanceCoordToActiveMapCoord(
          travel.destinationMapName, travel.destinationX, travel.destinationY);
      if (worldLoc.valid) {
        WorldSpawnPlayerAtXY(worldLoc.x, worldLoc.y).execute(state);
      } else {
        WorldSpawnPlayerAtXY(travel.destinationX, travel.destinationY).execute(state);
      }
    }
  }

public:
  explicit WorldTravel(model::TravelTrigger _travel) : travel(std::move(_travel)) {}
};

} // namespace actions

} // namespace state
