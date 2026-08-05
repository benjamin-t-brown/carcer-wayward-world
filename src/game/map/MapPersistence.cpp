#include "game/map/MapPersistence.h"
#include "bmin/StringInterop.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileFields.h"
#include "model/templates/CharacterTemplate.h"

namespace game {

void createMapInstances(state::State& state, const db::Database& database) {
  state.mapInstances = bmin::Map<bmin::String, model::MapInstance>{};

  // getMapTemplates() returns const Map&; bmin::Map iteration needs a non-const begin().
  auto& templates = const_cast<bmin::Map<bmin::String, model::CarcerMapTemplate>&>(
      database.getMapTemplates());
  for (auto it = templates.begin(); it != templates.end(); ++it) {
    model::MapInstance instance = model::createMapInstanceFromTemplate(it->value);
    for (size_t ci = 0; ci < instance.persistentState.characters.size(); ci++) {
      model::tryApplyCharacterTemplateToInstance(instance.persistentState.characters[ci],
                                                 database);
    }
    state.mapInstances[instance.templateName] = std::move(instance);
  }
}

void advanceWorldMovementTicks(state::State& state, int steps) {
  if (steps <= 0) {
    return;
  }
  state.playerMovementCount += steps;

  for (auto it = state.mapInstances.begin(); it != state.mapInstances.end(); ++it) {
    ageMapInstanceTileFields(it->value, steps);
    agePersistentTileFieldRecords(it->value.persistentState.tileFields, steps);
  }
}

void markMapCharacterDefeated(state::State& state,
                              const model::CharacterInstance& character) {
  if (state.world.activeMap.gridId.empty()) {
    return;
  }

  ActiveMapOrchestrator orch;
  orch.fetchMapGrid(state.world.activeMap.gridId);
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

  auto& grids = const_cast<bmin::Map<bmin::String, model::MapGridTemplate>&>(
      database.getMapGridTemplates());
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
    const auto& mapTemplate =
        database.getMapTemplate(bmin::toStringView(mapOrGridName));
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
