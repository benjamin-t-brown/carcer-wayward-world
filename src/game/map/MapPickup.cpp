#include "game/map/MapPickup.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapPathfinding.h"
#include "game/map/MapWalkability.h"

namespace game {

bool isActiveMapTileContainer(model::ActiveMap& activeMap,
                              int worldX,
                              int worldY,
                              const db::Database& database) {
  if (activeMap.gridId.empty()) {
    return false;
  }
  ActiveMapOrchestrator orch;
  orch.fetchMapGrid(activeMap.gridId);
  auto* map = orch.getMapInstanceAt(worldX, worldY);
  const auto local = orch.activeMapCoordToInstanceCoord(worldX, worldY);
  if (!map || !local.valid) {
    return false;
  }
  map->tileLayerNumber = activeMap.mapLayer;
  const auto* tile = tileAtCurrentLayer(*map, local.x, local.y);
  if (!tile) {
    return false;
  }
  return isTileEffectivelyContainer(*tile, database);
}

bmin::DynArray<model::ItemInstance>
collectItemsAtActiveMapTile(const model::ActiveMap& activeMap, int worldX, int worldY) {
  bmin::DynArray<model::ItemInstance> items;
  for (size_t i = 0; i < activeMap.items.size(); i++) {
    const auto& item = activeMap.items[i];
    if (item.x == worldX && item.y == worldY) {
      items.pushBack(item);
    }
  }
  return items;
}

bmin::DynArray<model::ItemInstance>
collectItemsWithinPickupRange(model::ActiveMap& activeMap,
                              const model::CharacterInstance& character,
                              int maxSteps,
                              const db::Database& database) {
  bmin::DynArray<model::ItemInstance> items;
  const auto reachable =
      collectReachableTiles(activeMap, character, maxSteps, database);
  for (size_t i = 0; i < activeMap.items.size(); i++) {
    const auto& item = activeMap.items[i];
    if (!isTileInReachableSet(reachable, item.x, item.y)) {
      continue;
    }
    if (isActiveMapTileContainer(activeMap, item.x, item.y, database)) {
      continue;
    }
    items.pushBack(item);
  }
  return items;
}

} // namespace game
