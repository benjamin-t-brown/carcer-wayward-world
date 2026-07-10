#pragma once

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

    auto& player = state->player;
    if (player.party.empty()) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: party is empty" << LOG_ENDL;
      return;
    }

    auto partyIndex = player.currentPartyMemberIndex;
    if (partyIndex < 0 ||
        static_cast<size_t>(partyIndex) >= player.party.size()) {
      partyIndex = 0;
    }
    const auto& member = player.party[static_cast<size_t>(partyIndex)];

    auto tile = model::tileIndexToXY(marker->i, map.width > 0 ? map.width : mapTemplate.width);

    map.characters.eraseIf([&](const model::CharacterInstance& c) {
      return c.id == member.instanceId;
    });

    auto instance = model::CharacterInstance{};
    instance.id = member.instanceId;
    instance.name = member.name.empty() ? member.params.name : member.name;
    instance.templateName =
        member.templateName.empty() ? member.params.name : member.templateName;
    instance.x = tile.x;
    instance.y = tile.y;
    map.characters.pushBack(std::move(instance));
  }

public:
  explicit WorldSpawnPlayerAtMarker(bmin::String _markerName = "MarkerPlayer")
      : markerName(std::move(_markerName)) {}
};

} // namespace actions

} // namespace state
