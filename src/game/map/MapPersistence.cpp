#include "game/map/MapPersistence.h"
#include "bmin/StringInterop.h"
#include "game/map/MapVision.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileFields.h"


namespace game {
namespace {

int tileCellIndex(const model::MapInstance& map, int x, int y) {
  if (map.width <= 0 || x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return -1;
  }
  return y * map.width + x;
}

model::TileInstance* findTileAtCell(model::MapInstance& map, int layer, int x, int y) {
  const auto index = tileCellIndex(map, x, y);
  if (index < 0) {
    return nullptr;
  }
  const auto* layerTiles = model::mapLayerPtr(map.tiles, layer);
  if (!layerTiles || index >= static_cast<int>(layerTiles->size())) {
    return nullptr;
  }
  return const_cast<model::TileInstance*>(&(*layerTiles)[static_cast<size_t>(index)]);
}

bmin::DynArray<model::PersistentTileFieldRecord>
captureTileFields(const model::MapInstance& map) {
  auto records = bmin::DynArray<model::PersistentTileFieldRecord>{};
  for (size_t layerIdx = 0; layerIdx < map.tiles.size(); layerIdx++) {
    const auto& layer = map.tiles[layerIdx];
    for (size_t ti = 0; ti < layer.size(); ti++) {
      const auto& tile = layer[ti];
      if (tile.fields.empty()) {
        continue;
      }
      auto record = model::PersistentTileFieldRecord{};
      record.layer = static_cast<int>(layerIdx);
      record.x = tile.x;
      record.y = tile.y;
      record.fields = tile.fields;
      records.pushBack(std::move(record));
    }
  }
  return records;
}

void applyTileFields(model::MapInstance& map,
                     const bmin::DynArray<model::PersistentTileFieldRecord>& records) {
  for (const auto& record : records) {
    auto* tile = findTileAtCell(map, record.layer, record.x, record.y);
    if (tile == nullptr) {
      continue;
    }
    tile->fields = record.fields;
  }
}

void applyDefeatedCharacters(
    model::MapInstance& map,
    const bmin::DynArray<model::DefeatedCharacterRecord>& defeated) {
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

void flushMapInstance(const model::MapInstance& map,
                      model::PersistentMapState& persistent,
                      const db::Database& database) {
  persistent.explored = captureExploredMask(map);
  persistent.openedDoors = captureOpenedDoors(map, database);
  persistent.tileFields = captureTileFields(map);
}

void hydrateMapInstance(model::MapInstance& map,
                        const model::PersistentMapState& persistent,
                        const db::Database& /*database*/) {
  // Structural mutations first, then explored (visibility is recomputed on spawn/move).
  applyOpenedDoors(map, persistent.openedDoors);
  applyTileFields(map, persistent.tileFields);
  applyDefeatedCharacters(map, persistent.defeatedCharacters);
  applyExploredMask(map, persistent.explored);
}

void markMapCharacterDefeated(
    model::World& world,
    bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
    const model::CharacterInstance& character) {
  const auto& map = world.currentMap;
  if (map.templateName.empty()) {
    return;
  }

  const auto spawnX = character.spawnX >= 0 ? character.spawnX : character.x;
  const auto spawnY = character.spawnY >= 0 ? character.spawnY : character.y;

  auto& persistent = mapsByTemplate[map.templateName];
  for (const auto& existing : persistent.defeatedCharacters) {
    if (existing.templateName == character.templateName && existing.x == spawnX &&
        existing.y == spawnY) {
      return;
    }
  }

  auto record = model::DefeatedCharacterRecord{};
  record.templateName = character.templateName;
  record.x = spawnX;
  record.y = spawnY;
  persistent.defeatedCharacters.pushBack(std::move(record));
}

void flushCurrentMapToPersistence(
    model::World& world,
    bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
    const db::Database& database) {
  const auto& map = world.currentMap;
  if (map.templateName.empty() || map.width <= 0 || map.height <= 0) {
    return;
  }
  flushMapInstance(map, mapsByTemplate[map.templateName], database);
}

void hydrateCurrentMapFromPersistence(
    model::World& world,
    bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
    const db::Database& database) {
  auto& map = world.currentMap;
  if (map.templateName.empty()) {
    return;
  }
  if (!mapsByTemplate.contains(map.templateName)) {
    return;
  }
  hydrateMapInstance(map, mapsByTemplate[map.templateName], database);
}

void enterMap(model::World& world,
              bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
              const bmin::String& templateName,
              const db::Database& database) {
  flushCurrentMapToPersistence(world, mapsByTemplate, database);

  const auto& mapTemplate = database.getMapTemplate(bmin::toStringView(templateName));
  world.currentMap = model::createMapInstanceFromTemplate(mapTemplate);
  world.name = mapTemplate.label.empty() ? mapTemplate.name : mapTemplate.label;
  world.camera.camX = 0;
  world.camera.camY = 0;
  world.camera.cameraMode = model::CameraMode::Follow;
  world.camera.cameraFollowCharacterId = bmin::String{};
  world.damageParticles.clear();

  hydrateCurrentMapFromPersistence(world, mapsByTemplate, database);
}

void advanceWorldMovementTicks(
    model::World& world,
    bmin::Map<bmin::String, model::PersistentMapState>& mapsByTemplate,
    int steps,
    const db::Database& database) {
  if (steps <= 0) {
    return;
  }
  world.playerMovementCount += steps;

  flushCurrentMapToPersistence(world, mapsByTemplate, database);

  const auto& currentTemplate = world.currentMap.templateName;
  for (auto it = mapsByTemplate.begin(); it != mapsByTemplate.end(); ++it) {
    if (it->key == currentTemplate) {
      continue;
    }
    agePersistentTileFieldRecords(it->value.tileFields, steps);
  }

  if (!currentTemplate.empty()) {
    auto& persistent = mapsByTemplate[currentTemplate];
    agePersistentTileFieldRecords(persistent.tileFields, steps);
    applyTileFields(world.currentMap, persistent.tileFields);
  } else {
    ageMapInstanceTileFields(world.currentMap, steps);
  }
}

} // namespace game
