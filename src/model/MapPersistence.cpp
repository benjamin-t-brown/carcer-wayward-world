#include "model/MapPersistence.h"
#include "model/MapVision.h"
#include "model/MapWalkability.h"
#include "bmin/StringInterop.h"

namespace model {

void flushMapInstance(const MapInstance& map,
                      PersistentMapState& persistent,
                      const db::Database& database) {
  persistent.explored = captureExploredMask(map);
  persistent.openedDoors = captureOpenedDoors(map, database);
}

void hydrateMapInstance(MapInstance& map,
                        const PersistentMapState& persistent,
                        const db::Database& /*database*/) {
  // Structural mutations first, then explored (visibility is recomputed on spawn/move).
  applyOpenedDoors(map, persistent.openedDoors);
  applyExploredMask(map, persistent.explored);
}

void flushCurrentMapToPersistence(World& world, const db::Database& database) {
  const auto& map = world.currentMap;
  if (map.templateName.empty() || map.width <= 0 || map.height <= 0) {
    return;
  }
  flushMapInstance(map, world.mapsByTemplate[map.templateName], database);
}

void hydrateCurrentMapFromPersistence(World& world, const db::Database& database) {
  auto& map = world.currentMap;
  if (map.templateName.empty()) {
    return;
  }
  if (!world.mapsByTemplate.contains(map.templateName)) {
    return;
  }
  hydrateMapInstance(map, world.mapsByTemplate[map.templateName], database);
}

void enterMap(World& world, const bmin::String& templateName, const db::Database& database) {
  flushCurrentMapToPersistence(world, database);

  const auto& mapTemplate = database.getMapTemplate(bmin::toStringView(templateName));
  world.currentMap = createMapInstanceFromTemplate(mapTemplate);
  world.name = mapTemplate.label.empty() ? mapTemplate.name : mapTemplate.label;
  world.camX = 0;
  world.camY = 0;
  world.cameraMode = CameraMode::Follow;
  world.cameraFollowCharacterId = bmin::String{};

  hydrateCurrentMapFromPersistence(world, database);
}

} // namespace model
