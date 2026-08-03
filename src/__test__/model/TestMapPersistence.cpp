#include "db/Database.h"
#include "model/Combat.h"
#include "game/map/MapPersistence.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileFields.h"
#include "model/instances/MapInstance.h"
#include "model/instances/World.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"

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
    layer.pushBack(1);
  }
  mapTemplate.tiles.pushBack(std::move(layer));

  auto placement = model::MapCharacterPlacement{};
  placement.l = 0;
  placement.i = 4;
  placement.name = "slime";
  mapTemplate.characters.pushBack(std::move(placement));
  return mapTemplate;
}

model::CharacterInstance* findEnemy(model::MapInstance& map) {
  for (size_t i = 0; i < map.characters.size(); i++) {
    if (map.characters[i].templateName == "slime") {
      return &map.characters[i];
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

  model::World world;
  bmin::Map<bmin::String, model::PersistentMapState> mapsByTemplate;
  world.currentMap = model::createMapInstanceFromTemplate(mapTemplate);

  auto* enemy = findEnemy(world.currentMap);
  ok = assertTrue(enemy != nullptr, "enemy spawned from template") && ok;
  if (enemy) {
    enemy->x = 2;
    enemy->y = 1;
    game::markMapCharacterDefeated(world, mapsByTemplate, *enemy);
    for (size_t i = 0; i < world.currentMap.characters.size();) {
      if (world.currentMap.characters[i].templateName == "slime") {
        world.currentMap.characters.erase(i);
      } else {
        ++i;
      }
    }
  }

  game::addTileFieldAt(world.currentMap, 1, 1, game::TileFieldType::BLOOD);
  game::flushMapInstance(world.currentMap, mapsByTemplate["test_map"], database);

  world.currentMap = model::createMapInstanceFromTemplate(mapTemplate);
  game::hydrateMapInstance(world.currentMap, mapsByTemplate["test_map"], database);

  ok = assertTrue(findEnemy(world.currentMap) == nullptr, "defeated enemy stays removed") && ok;

  auto* tile = game::tileAtCurrentLayer(world.currentMap, 1, 1);
  ok = assertTrue(tile != nullptr, "tile with blood exists") && ok;
  if (tile) {
    ok = assertEqual(static_cast<int>(tile->fields.size()), 1, "blood field persisted") && ok;
    if (!tile->fields.empty()) {
      ok = assertTrue(tile->fields[0].type == game::TileFieldType::BLOOD, "blood field type") &&
           ok;
    }
  }

  if (ok) {
    LOG(INFO) << "TestMapPersistence PASSED" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << "TestMapPersistence FAILED" << LOG_ENDL;
  return 1;
}
