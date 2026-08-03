#include "game/map/ActiveMapOrchestrator.h"
#include "bmin/StringInterop.h"
#include <stdexcept>

namespace game {

const model::MapGridTemplate& ActiveMapOrchestrator::requireGrid() const {
  if (!grid) {
    throw std::runtime_error("ActiveMapOrchestrator: map grid is not loaded");
  }
  return *grid;
}

int ActiveMapOrchestrator::getMapLayerId(int mapLayerId) const {
  if (mapLayerId == USE_WORLD_MAP_LAYER) {
    return getStateManager()->getState().world.activeMap.mapLayer;
  }
  return mapLayerId;
}

ActiveMapLoc ActiveMapOrchestrator::findMapGridEntry(const bmin::String& mapName) const {
  const auto& g = requireGrid();
  for (size_t y = 0; y < g.cells.size(); ++y) {
    const auto& row = g.cells[y];
    for (size_t x = 0; x < row.size(); ++x) {
      if (row[x] == mapName) {
        return ActiveMapLoc{static_cast<int>(x), static_cast<int>(y), true};
      }
    }
  }
  return ActiveMapLoc{};
}

model::MapInstance* ActiveMapOrchestrator::getMapInstanceByName(
    const bmin::String& mapName) {
  auto& state = getStateManager()->getState();
  auto it = state.mapInstances.find(mapName);
  if (it == state.mapInstances.end()) {
    return nullptr;
  }
  return &it->value;
}

model::MapInstance* ActiveMapOrchestrator::getMapInstanceAtGrid(int gridX, int gridY) {
  const auto& g = requireGrid();
  if (gridX < 0 || gridX >= g.gridWidth || gridY < 0 || gridY >= g.gridHeight) {
    return nullptr;
  }
  int mapIndex = gridY * g.gridWidth + gridX;
  if (mapInstanceCache.contains(mapIndex)) {
    return mapInstanceCache[mapIndex];
  }
  auto map = g.cells[gridY][gridX];
  if (map.empty()) {
    return nullptr;
  }
  auto mapInstance = getMapInstanceByName(map);
  if (!mapInstance) {
    return nullptr;
  }
  mapInstanceCache.insert(mapIndex, mapInstance);
  return mapInstance;
}

ActiveMapOrchestrator::ActiveMapOrchestrator() {}

void ActiveMapOrchestrator::fetchMapGrid(const bmin::String& gridName) {
  auto* database = getDatabase();
  if (!database) {
    throw std::runtime_error("ActiveMapOrchestrator::loadMapGrid: database is nullptr");
  }
  grid = &database->getMapGridTemplate(bmin::toStringView(gridName));
}

const model::MapGridTemplate& ActiveMapOrchestrator::getMapGrid() const {
  return requireGrid();
}

ActiveMapLoc ActiveMapOrchestrator::getMapOffset(const bmin::String& mapName) const {
  const ActiveMapLoc cell = findMapGridEntry(mapName);
  if (!cell.valid) {
    return ActiveMapLoc{};
  }
  const auto& g = requireGrid();
  return ActiveMapLoc{cell.x * g.mapWidth, cell.y * g.mapHeight, true};
}

ActiveMapLoc ActiveMapOrchestrator::activeMapCoordToInstanceCoord(int worldX,
                                                                  int worldY) const {
  const auto& g = requireGrid();
  if (worldX < 0 || worldY < 0 || worldX >= g.mapWidth * g.gridWidth ||
      worldY >= g.mapHeight * g.gridHeight) {
    return ActiveMapLoc{};
  }
  int gridX = worldX / g.mapWidth;
  int gridY = worldY / g.mapHeight;
  return ActiveMapLoc{worldX - gridX * g.mapWidth, worldY - gridY * g.mapHeight, true};
}

ActiveMapLoc ActiveMapOrchestrator::instanceCoordToActiveMapCoord(
    const bmin::String& mapName, int mapX, int mapY) const {
  const ActiveMapLoc offset = getMapOffset(mapName);
  if (!offset.valid) {
    return ActiveMapLoc{};
  }
  return ActiveMapLoc{offset.x + mapX, offset.y + mapY, true};
}

ActiveMapLoc ActiveMapOrchestrator::getGridSize() const {
  const auto& g = requireGrid();
  return ActiveMapLoc{g.gridWidth, g.gridHeight, true};
}

ActiveMapLoc ActiveMapOrchestrator::getTotalMapTilesSize() const {
  const auto& g = requireGrid();
  return ActiveMapLoc{g.mapWidth * g.mapHeight, g.mapWidth * g.mapHeight, true};
}

model::MapInstance* ActiveMapOrchestrator::getDefaultMapInstance() {
  const auto& g = requireGrid();
  return getMapInstanceByName(g.cells[0][0]);
}

model::MapInstance* ActiveMapOrchestrator::getMapInstanceAt(int worldX, int worldY) {
  const auto& g = requireGrid();
  if (worldX < 0 || worldY < 0 || worldX >= g.mapWidth * g.gridWidth ||
      worldY >= g.mapHeight * g.gridHeight) {
    return nullptr;
  }

  int gridX = worldX / g.mapWidth;
  int gridY = worldY / g.mapHeight;
  if (gridX < 0 || gridX >= g.gridWidth || gridY < 0 || gridY >= g.gridHeight) {
    return nullptr;
  }
  int mapIndex = gridY * g.gridWidth + gridX;

  if (mapInstanceCache.contains(mapIndex)) {
    return mapInstanceCache[mapIndex];
  }

  return getMapInstanceAtGrid(gridX, gridY);
}

model::CharacterInstance* ActiveMapOrchestrator::findCharacterById(
    const bmin::String& characterId, int /*mapLayerId*/) {
  auto& world = getStateManager()->getState().world;
  for (auto it = world.activeMap.characters.begin();
       it != world.activeMap.characters.end();
       ++it) {
    if (it->id == characterId) {
      return it;
    }
  }
  return nullptr;
}

model::CharacterInstance* ActiveMapOrchestrator::findCharacterAt(int worldX,
                                                                 int worldY,
                                                                 int mapLayerId) {
  auto& world = getStateManager()->getState().world;
  for (auto it = world.activeMap.characters.begin();
       it != world.activeMap.characters.end();
       ++it) {
    if (it->x == worldX && it->y == worldY) {
      return it;
    }
  }
  return nullptr;
}

model::TileInstance* ActiveMapOrchestrator::findTileAt(int worldX,
                                                       int worldY,
                                                       int mapLayerId) {
  auto map = getMapInstanceAt(worldX, worldY);
  if (!map) {
    return nullptr;
  }

  const ActiveMapLoc local = activeMapCoordToInstanceCoord(worldX, worldY);
  if (!local.valid || local.x < 0 || local.y < 0 || local.x >= map->width ||
      local.y >= map->height) {
    return nullptr;
  }

  const int layerNum = getMapLayerId(mapLayerId);
  const int index = local.y * map->width + local.x;
  if (!map->persistentState.tiles.contains(layerNum)) {
    return nullptr;
  }
  auto layerTiles = map->persistentState.tiles[layerNum];
  if (index >= static_cast<int>(layerTiles.size())) {
    return nullptr;
  }
  return model::mapInstanceGetTileAt(*map, local.x, local.y, layerNum);
}

ActiveMapMarker ActiveMapOrchestrator::findMarker(const bmin::String& mapName,
                                                  const bmin::String& markerName) {
  if (markerName.empty()) {
    return ActiveMapMarker{};
  }

  if (grid == nullptr) {
    return ActiveMapMarker{};
  }

  auto mapInstance = getMapInstanceByName(mapName);
  if (!mapInstance) {
    return ActiveMapMarker{};
  }

  auto* database = getDatabase();
  auto& mapTemplate =
      database->getMapTemplate(bmin::toStringView(mapInstance->templateName));
  auto marker = model::findMarkerOnTemplate(mapTemplate, markerName);
  if (!marker) {
    return ActiveMapMarker{};
  }
  auto layer = marker->l;
  auto index = marker->i;
  auto x = index % mapInstance->width;
  auto y = index / mapInstance->width;
  auto activeMapLoc = instanceCoordToActiveMapCoord(mapName, x, y);
  if (!activeMapLoc.valid) {
    return ActiveMapMarker{};
  }
  return ActiveMapMarker{marker->name, activeMapLoc.x, activeMapLoc.y, layer, true};
}

} // namespace game
