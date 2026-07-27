#include "model/MapPersistence.h"
#include "model/MapVision.h"
#include "model/MapWalkability.h"
#include "bmin/StringInterop.h"

namespace model {
namespace {

int tileCellIndex(const MapInstance& map, int x, int y) {
  if (map.width <= 0 || x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return -1;
  }
  return y * map.width + x;
}

TileInstance* findTileAtCell(MapInstance& map, int layer, int x, int y) {
  const auto index = tileCellIndex(map, x, y);
  if (index < 0) {
    return nullptr;
  }
  const auto* layerTiles = mapLayerPtr(map.tiles, layer);
  if (!layerTiles || index >= static_cast<int>(layerTiles->size())) {
    return nullptr;
  }
  return const_cast<TileInstance*>(&(*layerTiles)[static_cast<size_t>(index)]);
}

bmin::DynArray<PersistentTileFieldRecord> captureTileFields(const MapInstance& map) {
  auto records = bmin::DynArray<PersistentTileFieldRecord>{};
  for (size_t layerIdx = 0; layerIdx < map.tiles.size(); layerIdx++) {
    const auto& layer = map.tiles[layerIdx];
    for (size_t ti = 0; ti < layer.size(); ti++) {
      const auto& tile = layer[ti];
      if (tile.fields.empty()) {
        continue;
      }
      auto record = PersistentTileFieldRecord{};
      record.layer = static_cast<int>(layerIdx);
      record.x = tile.x;
      record.y = tile.y;
      record.fields = tile.fields;
      records.pushBack(std::move(record));
    }
  }
  return records;
}

void applyTileFields(MapInstance& map, const bmin::DynArray<PersistentTileFieldRecord>& records) {
  for (const auto& record : records) {
    auto* tile = findTileAtCell(map, record.layer, record.x, record.y);
    if (tile == nullptr) {
      continue;
    }
    tile->fields = record.fields;
  }
}

void applyDefeatedCharacters(MapInstance& map,
                             const bmin::DynArray<DefeatedCharacterRecord>& defeated) {
  if (defeated.empty()) {
    return;
  }

  for (size_t i = 0; i < map.characters.size();) {
    const auto& character = map.characters[i];
    auto remove = false;
    for (const auto& record : defeated) {
      if (character.templateName != record.templateName) {
        continue;
      }
      const auto spawnX = character.spawnX >= 0 ? character.spawnX : character.x;
      const auto spawnY = character.spawnY >= 0 ? character.spawnY : character.y;
      if (spawnX == record.x && spawnY == record.y) {
        remove = true;
        break;
      }
    }
    if (remove) {
      map.characters.erase(i);
    } else {
      ++i;
    }
  }
}

} // namespace

void flushMapInstance(const MapInstance& map,
                      PersistentMapState& persistent,
                      const db::Database& database) {
  persistent.explored = captureExploredMask(map);
  persistent.openedDoors = captureOpenedDoors(map, database);
  persistent.tileFields = captureTileFields(map);
}

void hydrateMapInstance(MapInstance& map,
                        const PersistentMapState& persistent,
                        const db::Database& /*database*/) {
  // Structural mutations first, then explored (visibility is recomputed on spawn/move).
  applyOpenedDoors(map, persistent.openedDoors);
  applyTileFields(map, persistent.tileFields);
  applyDefeatedCharacters(map, persistent.defeatedCharacters);
  applyExploredMask(map, persistent.explored);
}

void markMapCharacterDefeated(World& world, const CharacterInstance& character) {
  const auto& map = world.currentMap;
  if (map.templateName.empty()) {
    return;
  }

  const auto spawnX = character.spawnX >= 0 ? character.spawnX : character.x;
  const auto spawnY = character.spawnY >= 0 ? character.spawnY : character.y;

  auto& persistent = world.mapsByTemplate[map.templateName];
  for (const auto& existing : persistent.defeatedCharacters) {
    if (existing.templateName == character.templateName && existing.x == spawnX &&
        existing.y == spawnY) {
      return;
    }
  }

  auto record = DefeatedCharacterRecord{};
  record.templateName = character.templateName;
  record.x = spawnX;
  record.y = spawnY;
  persistent.defeatedCharacters.pushBack(std::move(record));
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
  world.camera.camX = 0;
  world.camera.camY = 0;
  world.camera.cameraMode = CameraMode::Follow;
  world.camera.cameraFollowCharacterId = bmin::String{};
  world.damageParticles.clear();

  hydrateCurrentMapFromPersistence(world, database);
}

} // namespace model
