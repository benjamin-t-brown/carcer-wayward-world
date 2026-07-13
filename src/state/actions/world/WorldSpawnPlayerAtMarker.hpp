#pragma once

#include "model/MapVision.h"
#include "model/TileTriggers.h"
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
// the current map template. Does not move the camera. NPC/item template
// placements remain a follow-up.
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

    auto& map = state->world.currentMap;
    if (map.templateName.empty()) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: no current map loaded" << LOG_ENDL;
      return;
    }

    const auto& mapTemplate =
        database->getMapTemplate(bmin::toStringView(map.templateName));
    const auto* marker = model::findMarkerOnTemplate(mapTemplate, markerName);
    if (!marker) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: marker not found: " << markerName
                 << LOG_ENDL;
      return;
    }

    auto tile = model::tileIndexToXY(marker->i, map.width > 0 ? map.width : mapTemplate.width);
    map.tileLayerNumber = marker->l;

    if (!model::placePartyAvatarAt(map, state->player, tile.x, tile.y)) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: party is empty" << LOG_ENDL;
      return;
    }

    model::updateMapVisibilityFromPlayer(map, tile.x, tile.y, *database);
  }

public:
  explicit WorldSpawnPlayerAtMarker(bmin::String _markerName = "MarkerPlayer")
      : markerName(std::move(_markerName)) {}
};

} // namespace actions

} // namespace state
