module;
#include <cstddef>

export module carcer.actions.world:WorldSpawnPlayer;
export import carcer.state;
import :WorldLoadActiveMap;
import :WorldSpawnPlayerAtMarker;
import carcer.game.map;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class WorldSpawnPlayer : public AbstractAction {
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

} // export
