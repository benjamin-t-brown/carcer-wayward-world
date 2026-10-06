#include "game/map/MapPersistence.h"
#include "bmin/StringInterop.h"
#include "game/combat/DropTables.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/CharacterConstruction.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileFields.h"
#include "sdl2w/Logger.h"

namespace game {

void materializeMapDropTablePlacements(model::MapInstance& instance,
                                         const model::CarcerMapTemplate& mapTemplate,
                                         const db::Database& database) {
  for (const auto& placement : mapTemplate.items) {
    if (placement.dropTable.empty()) {
      continue;
    }
    const auto tile = model::tileIndexToXY(placement.i, instance.width);
    bmin::DynArray<DropRollResult> dropRolls;
    rollDropTable(bmin::toStringView(placement.dropTable), database, dropRolls);
    for (const auto& roll : dropRolls) {
      appendDropRollToItems(instance.persistentState.items, roll, tile.x, tile.y);
    }
  }
}

MapInstanceStore createMapInstances(const db::Database& database) {
  auto mapInstances = MapInstanceStore{};

  const auto& templates = database.getMapTemplates();
  for (auto it = templates.begin(); it != templates.end(); ++it) {
    model::MapInstance instance = model::createMapInstanceFromTemplate(it->value);
    materializeMapDropTablePlacements(instance, it->value, database);
    applyUnlockedDoors(instance, instance.persistentState.unlockedDoors);
    applyOpenedDoors(instance, instance.persistentState.openedDoors);
    applyChangedTiles(instance, instance.persistentState.changedTiles, &database);
    for (size_t ci = 0; ci < instance.persistentState.characters.size(); ci++) {
      applyCharacterTemplateFromDatabase(instance.persistentState.characters[ci],
                                         database);
    }
    auto& layers = model::mapInstanceTiles(instance);
    for (auto& layer : layers) {
      auto& layerTiles = layer.value;
      for (auto& tile : layerTiles) {
        if (game::isTileEffectivelyContainer(tile, database)) {
          tile.isContainer = true;
        }
        if (game::isTileEffectivelyWalkable(tile, database)) {
          tile.isWalkable = true;
        }
      }
    }
    mapInstances[instance.templateName] = std::move(instance);
  }
  return mapInstances;
}

void restoreMapTilesFromTemplate(model::MapInstance& instance, const db::Database& database) {
  if (instance.templateName.empty()) {
    return;
  }
  const auto& templates = database.getMapTemplates();
  const auto it = templates.find(instance.templateName);
  if (it == templates.end()) {
    LOG(ERROR) << "restoreMapTilesFromTemplate: map template not found: "
               << instance.templateName << LOG_ENDL;
    return;
  }

  model::MapInstance fresh = model::createMapInstanceFromTemplate(it->value);

  auto openedDoors = std::move(instance.persistentState.openedDoors);
  auto unlockedDoors = std::move(instance.persistentState.unlockedDoors);
  auto changedTiles = std::move(instance.persistentState.changedTiles);
  auto explored = std::move(instance.persistentState.explored);
  auto defeatedCharacters = std::move(instance.persistentState.defeatedCharacters);
  auto tileFields = std::move(instance.persistentState.tileFields);
  auto characters = std::move(instance.persistentState.characters);
  auto items = std::move(instance.persistentState.items);
  const int version = instance.persistentState.version;

  instance.persistentState.tiles = std::move(fresh.persistentState.tiles);
  instance.persistentState.openedDoors = std::move(openedDoors);
  instance.persistentState.unlockedDoors = std::move(unlockedDoors);
  instance.persistentState.changedTiles = std::move(changedTiles);
  instance.persistentState.explored = std::move(explored);
  instance.persistentState.defeatedCharacters = std::move(defeatedCharacters);
  instance.persistentState.tileFields = std::move(tileFields);
  instance.persistentState.characters = std::move(characters);
  instance.persistentState.items = std::move(items);
  instance.persistentState.version = version;

  applyUnlockedDoors(instance, instance.persistentState.unlockedDoors);
  applyOpenedDoors(instance, instance.persistentState.openedDoors);
  applyChangedTiles(instance, instance.persistentState.changedTiles, &database);

  auto& layers = model::mapInstanceTiles(instance);
  for (auto& layer : layers) {
    auto& layerTiles = layer.value;
    for (auto& tile : layerTiles) {
      tile.isContainer = game::isTileEffectivelyContainer(tile, database);
      tile.isWalkable = game::isTileEffectivelyWalkable(tile, database);
    }
  }
}

void ageMapInstances(MapInstanceStore& mapInstances, int steps) {
  if (steps <= 0) {
    return;
  }

  for (auto it = mapInstances.begin(); it != mapInstances.end(); ++it) {
    ageMapInstanceTileFields(it->value, steps);
    agePersistentTileFieldRecords(it->value.persistentState.tileFields, steps);
  }
}

void markMapCharacterDefeated(model::ActiveMap& activeMap,
                              MapInstanceStore& mapInstances,
                              const model::CharacterInstance& character,
                              const db::Database& database) {
  if (activeMap.gridId.empty()) {
    return;
  }

  ActiveMapOrchestrator orch(activeMap, mapInstances, &database);
  auto* map = orch.getMapInstanceAt(character.x, character.y);
  if (!map) {
    map = orch.getDefaultMapInstance();
  }
  if (!map) {
    return;
  }

  const auto spawnX = character.spawnX >= 0 ? character.spawnX : character.x;
  const auto spawnY = character.spawnY >= 0 ? character.spawnY : character.y;
  // Convert world spawn to local if the character was on the active map.
  auto local = orch.activeMapCoordToInstanceCoord(spawnX, spawnY);
  const int recordX = local.valid ? local.x : spawnX;
  const int recordY = local.valid ? local.y : spawnY;

  for (const auto& existing : map->persistentState.defeatedCharacters) {
    if (existing.templateName == character.templateName && existing.x == recordX &&
        existing.y == recordY) {
      return;
    }
  }

  auto record = model::DefeatedCharacterRecord{};
  record.templateName = character.templateName;
  record.x = recordX;
  record.y = recordY;
  map->persistentState.defeatedCharacters.pushBack(std::move(record));
}

bmin::String resolveGridIdForMapOrGrid(db::Database& database,
                                       const bmin::String& mapOrGridName) {
  if (mapOrGridName.empty()) {
    return bmin::String{};
  }
  if (database.findMapGridTemplate(bmin::toStringView(mapOrGridName))) {
    return mapOrGridName;
  }

  const auto& grids = database.getMapGridTemplates();
  for (auto it = grids.begin(); it != grids.end(); ++it) {
    const auto& grid = it->value;
    for (size_t y = 0; y < grid.cells.size(); ++y) {
      for (size_t x = 0; x < grid.cells[y].size(); ++x) {
        if (grid.cells[y][x] == mapOrGridName) {
          return grid.name;
        }
      }
    }
  }

  // Standalone map: ensure a 1x1 grid exists so ActiveMapOrchestrator can load it.
  try {
    const auto& mapTemplate = database.getMapTemplate(bmin::toStringView(mapOrGridName));
    model::MapGridTemplate grid;
    grid.name = mapOrGridName;
    grid.label = mapTemplate.label.empty() ? mapTemplate.name : mapTemplate.label;
    grid.gridWidth = 1;
    grid.gridHeight = 1;
    grid.mapWidth = mapTemplate.width > 0 ? mapTemplate.width : 1;
    grid.mapHeight = mapTemplate.height > 0 ? mapTemplate.height : 1;
    grid.cells = {{mapOrGridName}};
    database.addMapGridTemplate(grid);
    return mapOrGridName;
  } catch (...) {
    return bmin::String{};
  }
}

} // namespace game
