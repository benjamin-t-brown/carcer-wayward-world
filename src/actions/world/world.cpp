module;
#include <cstddef>
#include <cstdlib>

module carcer.actions;
import sdl2w;
import bmin.string_interop;
import carcer.model.templates;
import carcer.model.instances;
import carcer.game.map;
#include "macros.h"

namespace state {

namespace actions {

void WorldExamineAt::act() {
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

  game::ActiveMapOrchestrator orch;
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
      UiShowLayerPickUp showContainer(window, x, y);
      showContainer.execute(state);
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

} // namespace actions

} // namespace state
