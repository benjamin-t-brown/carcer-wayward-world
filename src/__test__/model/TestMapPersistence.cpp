#include "db/Database.h"
#include "game/map/MapPersistence.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileFields.h"
#include "game/map/TileTriggers.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/MapGrids.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/actions/world/WorldLoadActiveMap.hpp"

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

model::TileMetadata makeMeta(int id, bool walkable) {
  auto meta = model::TileMetadata{};
  meta.id = id;
  meta.isWalkable = walkable;
  meta.isSeeThrough = walkable;
  return meta;
}

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  tileset.tiles.pushBack(makeMeta(0, true));
  database.addTilesetTemplate(tileset);
}

void addEnemyTemplate(db::Database& database) {
  auto character = model::CharacterTemplate{};
  character.type = model::CharacterTemplateType::ENEMY;
  character.name = "slime";
  character.combat.hp = 20;
  database.addCharacterTemplate(character);
}

model::CarcerMapTemplate makeMapTemplate() {
  auto mapTemplate = model::CarcerMapTemplate{};
  mapTemplate.name = "test_map";
  mapTemplate.label = "Test Map";
  mapTemplate.width = 3;
  mapTemplate.height = 3;
  mapTemplate.spriteWidth = 28;
  mapTemplate.spriteHeight = 32;
  mapTemplate.tilesets.pushBack("test_terrain");

  auto layer = bmin::DynArray<int>{};
  for (auto i = 0; i < 9; i++) {
    layer.pushBack(0);
    layer.pushBack(0);
  }
  mapTemplate.tiles.pushBack(std::move(layer));

  auto placement = model::MapCharacterPlacement{};
  placement.l = 0;
  placement.i = 4;
  placement.name = "slime";
  mapTemplate.characters.pushBack(std::move(placement));
  return mapTemplate;
}

model::CharacterInstance* findEnemyOnActive(model::ActiveMap& activeMap) {
  for (size_t i = 0; i < activeMap.characters.size(); i++) {
    if (activeMap.characters[i].templateName == "slime") {
      return &activeMap.characters[i];
    }
  }
  return nullptr;
}

} // namespace

int main() {
  LOG(INFO) << "Starting TestMapPersistence" << LOG_ENDL;
  auto ok = true;

  db::Database database;
  addTestTileset(database);
  addEnemyTemplate(database);
  state::DatabaseInterface::setDatabase(&database);

  auto mapTemplate = makeMapTemplate();
  database.addMapTemplate(mapTemplate);

  model::MapGridTemplate grid;
  grid.name = "test_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 3;
  grid.mapHeight = 3;
  grid.cells = {{"test_map"}};
  database.addMapGridTemplate(grid);

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  auto& state = stateManager.getState();

  game::createMapInstances(state, database);
  state::actions::WorldLoadActiveMap("test_grid").execute(&state);

  auto* enemy = findEnemyOnActive(state.world.activeMap);
  ok = assertTrue(enemy != nullptr, "enemy hoisted from template") && ok;
  if (enemy) {
    enemy->x = 2;
    enemy->y = 1;
    enemy->spawnX = 1;
    enemy->spawnY = 1;
    game::markMapCharacterDefeated(state, *enemy);
    for (size_t i = 0; i < state.world.activeMap.characters.size();) {
      if (state.world.activeMap.characters[i].templateName == "slime") {
        state.world.activeMap.characters.erase(i);
      } else {
        ++i;
      }
    }
  }

  game::addTileFieldAt(state.mapInstances["test_map"], 1, 1, game::TileFieldType::BLOOD);

  // Re-seed persistent characters as if the map template entities returned; hoist
  // must drop the defeated slime via persistentState.defeatedCharacters.
  {
    auto resurrected = model::CharacterInstance{};
    resurrected.id = "slime-again";
    resurrected.templateName = "slime";
    resurrected.x = 1;
    resurrected.y = 1;
    resurrected.spawnX = 1;
    resurrected.spawnY = 1;
    state.mapInstances["test_map"].persistentState.characters.pushBack(
        std::move(resurrected));
  }

  state::actions::WorldLoadActiveMap("test_grid").execute(&state);

  ok = assertTrue(findEnemyOnActive(state.world.activeMap) == nullptr,
                  "defeated enemy filtered on hoist") &&
       ok;

  auto* tile = game::tileAtCurrentLayer(state.mapInstances["test_map"], 1, 1);
  ok = assertTrue(tile != nullptr, "tile with blood exists") && ok;
  if (tile) {
    ok = assertEqual(static_cast<int>(tile->fields.size()), 1, "blood field persisted") &&
         ok;
    if (!tile->fields.empty()) {
      ok = assertTrue(tile->fields[0].type == game::TileFieldType::BLOOD, "blood field type") &&
           ok;
    }
  }

  ok = assertTrue(
      !state.mapInstances["test_map"].persistentState.defeatedCharacters.empty(),
      "defeated record stored on MapInstance") &&
       ok;

  if (ok) {
    LOG(INFO) << "TestMapPersistence PASSED" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << "TestMapPersistence FAILED" << LOG_ENDL;
  return 1;
}
