#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapVision.h"
#include "game/map/TileTriggers.h"
#include "model/instances/Player.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"

namespace state {

namespace actions {

// Spawns (or re-spawns) one player avatar CharacterInstance at a named marker on
// the active map grid. Does not move the camera.
class WorldSpawnPlayerAtMarker : public AbstractAction {
  bmin::String markerName;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: no active map loaded" << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch;
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto& grid = orch.getMapGrid();

    game::ActiveMapMarker found{};
    for (int y = 0; y < grid.gridHeight && !found.valid; ++y) {
      for (int x = 0; x < grid.gridWidth && !found.valid; ++x) {
        const auto& mapName = grid.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
        if (mapName.empty()) {
          continue;
        }
        found = orch.findMarker(mapName, markerName);
      }
    }

    if (!found.valid) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: marker not found: " << markerName
                 << LOG_ENDL;
      return;
    }

    world.activeMap.mapLayer = found.layer;

    if (!game::placePartyAvatarAt(world.activeMap, state->player, found.x, found.y)) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: party is empty" << LOG_ENDL;
      return;
    }

    game::updateActiveMapVisibilityFromPlayer(world, found.x, found.y, *database);
  }

public:
  explicit WorldSpawnPlayerAtMarker(bmin::String _markerName = "MarkerPlayer")
      : markerName(std::move(_markerName)) {}
};

} // namespace actions

} // namespace state
