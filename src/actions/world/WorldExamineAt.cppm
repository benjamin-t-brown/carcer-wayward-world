module;
#include <cmath>
#include <cstddef>

export module carcer.actions.world:WorldExamineAt;
export import carcer.state;
import sdl2w;
import carcer.game.map;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

class WorldExamineAt : public AbstractAction {
  sdl2w::Window* window = nullptr;
  int x = 0;
  int y = 0;

  static bool isAdjacentOrSame(int ax, int ay, int bx, int by) {
    return std::abs(ax - bx) <= 1 && std::abs(ay - by) <= 1;
  }

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldExamineAt::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldExamineAt::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, getDatabase());
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* map = orch.getMapInstanceAt(x, y);
    const auto local = orch.activeMapCoordToInstanceCoord(x, y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      LOG(INFO) << "You can't see there." << LOG_ENDL;
      return;
    }

    world.actionMode = model::WorldActionMode::NONE;
    world.actionAimTile.reset();

    const auto* tile = game::tileAtCurrentLayer(*map, local.x, local.y);
    if (tile && tile->eventTrigger && tile->eventTrigger->requiresLook) {
      state->triggers.pendingSpecialEventId = tile->eventTrigger->eventId;
      return;
    }

    const bool isContainer =
        tile != nullptr && game::isTileEffectivelyContainer(*tile, *database);
    if (isContainer) {
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
      const bool adjacent =
          avatar != nullptr && isAdjacentOrSame(avatar->x, avatar->y, x, y);
      if (adjacent && window) {
        const auto contents =
            game::collectItemsAtActiveMapTile(world.activeMap, x, y);
        if (contents.empty()) {
          LOG(INFO) << TRANSLATE("Nothing inside.") << LOG_ENDL;
          return;
        }
        pushLayerRequest(*state, LayerRequest{.id = LayerId::PickUp, .x = x, .y = y});
        return;
      }
      LOG(INFO) << game::formatExamineMessage(
                       *map, world.activeMap, x, y, local.x, local.y, *database)
                << LOG_ENDL;
      LOG(INFO) << TRANSLATE("You need to get closer to look inside.") << LOG_ENDL;
      return;
    }

    LOG(INFO) << game::formatExamineMessage(
                     *map, world.activeMap, x, y, local.x, local.y, *database)
              << LOG_ENDL;
  }

public:
  WorldExamineAt(int _x, int _y) : x(_x), y(_y) {}
  WorldExamineAt(sdl2w::Window* _window, int _x, int _y)
      : window(_window), x(_x), y(_y) {}
};

} // namespace actions

} // namespace state

} // export
