#include "game/map/MapPathfinding.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapWalkability.h"

namespace game {
namespace {

constexpr int NEIGHBOR_DX[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
constexpr int NEIGHBOR_DY[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

bool isVisited(const bmin::DynArray<PathTile>& visited, int x, int y) {
  for (size_t i = 0; i < visited.size(); i++) {
    if (visited[i].x == x && visited[i].y == y) {
      return true;
    }
  }
  return false;
}

bool isTilePathable(ActiveMapOrchestrator& orch,
                    model::ActiveMap& activeMap,
                    int x,
                    int y,
                    const bmin::String& characterId,
                    const db::Database& database) {
  auto* map = orch.getMapInstanceAt(x, y);
  const auto local = orch.activeMapCoordToInstanceCoord(x, y);
  if (!map || !local.valid) {
    return false;
  }
  map->tileLayerNumber = activeMap.mapLayer;
  if (!isDestinationWalkable(*map, local.x, local.y, database)) {
    return false;
  }
  for (size_t i = 0; i < activeMap.characters.size(); i++) {
    const auto& other = activeMap.characters[i];
    if (other.x == x && other.y == y && other.id != characterId) {
      return false;
    }
  }
  return true;
}

} // namespace

bool isTileInReachableSet(const bmin::DynArray<PathTile>& reachable, int x, int y) {
  return isVisited(reachable, x, y);
}

bmin::DynArray<PathTile> collectReachableTiles(model::ActiveMap& activeMap,
                                               int startX,
                                               int startY,
                                               int maxSteps,
                                               const bmin::String& characterId,
                                               const db::Database& database) {
  bmin::DynArray<PathTile> reachable;
  if (activeMap.gridId.empty() || maxSteps < 0) {
    return reachable;
  }

  ActiveMapOrchestrator orch;
  orch.fetchMapGrid(activeMap.gridId);
  const auto total = orch.getTotalMapTilesSize();
  if (!total.valid || total.x <= 0 || total.y <= 0) {
    return reachable;
  }
  if (startX < 0 || startY < 0 || startX >= total.x || startY >= total.y) {
    return reachable;
  }

  bmin::DynArray<PathTile> queue;
  reachable.pushBack({startX, startY, 0});
  queue.pushBack({startX, startY, 0});

  for (size_t qi = 0; qi < queue.size(); qi++) {
    const auto node = queue[qi];
    if (node.dist >= maxSteps) {
      continue;
    }
    for (int ni = 0; ni < 8; ni++) {
      const int nx = node.x + NEIGHBOR_DX[ni];
      const int ny = node.y + NEIGHBOR_DY[ni];
      if (nx < 0 || ny < 0 || nx >= total.x || ny >= total.y) {
        continue;
      }
      if (isVisited(reachable, nx, ny)) {
        continue;
      }
      if (!isTilePathable(orch, activeMap, nx, ny, characterId, database)) {
        continue;
      }
      const PathTile next{nx, ny, node.dist + 1};
      reachable.pushBack(next);
      queue.pushBack(next);
    }
  }

  return reachable;
}

bmin::DynArray<PathTile> collectReachableTiles(model::ActiveMap& activeMap,
                                               const model::CharacterInstance& character,
                                               int maxSteps,
                                               const db::Database& database) {
  return collectReachableTiles(
      activeMap, character.x, character.y, maxSteps, character.id, database);
}

} // namespace game
