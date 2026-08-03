#include "db/Database.h"
#include "game/map/MapPersistence.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileFields.h"
#include "model/instances/MapInstance.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"

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

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  auto meta = model::TileMetadata{};
  meta.id = 0;
  meta.isWalkable = true;
  meta.isSeeThrough = true;
  tileset.tiles.pushBack(meta);
  database.addTilesetTemplate(tileset);
}

model::CarcerMapTemplate makeMapTemplate(const bmin::String& name) {
  auto mapTemplate = model::CarcerMapTemplate{};
  mapTemplate.name = name;
  mapTemplate.label = name;
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
  return mapTemplate;
}

} // namespace

int main() {
  LOG(INFO) << "Starting TestTileFieldAging" << LOG_ENDL;
  auto ok = true;

  ok = assertEqual(game::tileFieldDefaultMoveDuration(game::TileFieldType::BLOOD),
                   game::TILE_FIELD_BLOOD_MOVE_DURATION,
                   "blood default duration") &&
       ok;
  ok = assertEqual(game::tileFieldDefaultMoveDuration(game::TileFieldType::STATIC),
                   0,
                   "static permanent") &&
       ok;

  {
    auto fields = bmin::DynArray<game::TileField>{};
    fields.pushBack(game::TileField{.type = game::TileFieldType::BLOOD, .moveDuration = 3});
    fields.pushBack(game::TileField{.type = game::TileFieldType::STATIC, .moveDuration = 0});
    game::ageTileFields(fields, 2);
    ok = assertEqual(static_cast<int>(fields.size()), 2, "static survives aging") && ok;
    ok = assertEqual(fields[0].moveDuration, 1, "blood duration decremented") && ok;
    game::ageTileFields(fields, 1);
    ok = assertEqual(static_cast<int>(fields.size()), 1, "expired blood removed") && ok;
    ok = assertTrue(fields[0].type == game::TileFieldType::STATIC, "static remains") && ok;
  }

  db::Database database;
  addTestTileset(database);
  state::DatabaseInterface::setDatabase(&database);

  database.addMapTemplate(makeMapTemplate("map_a"));
  database.addMapTemplate(makeMapTemplate("map_b"));

  state::State state;
  game::createMapInstances(state, database);

  game::addTileFieldAt(state.mapInstances["map_a"], 1, 1, game::TileFieldType::BLOOD);
  {
    auto* tile = game::tileAtCurrentLayer(state.mapInstances["map_a"], 1, 1);
    ok = assertTrue(tile != nullptr && tile->fields.size() == 1, "blood placed on map_a") &&
         ok;
  }

  // Age while "away" from map_a — advanceWorldMovementTicks ages all mapInstances.
  for (auto i = 0; i < game::TILE_FIELD_BLOOD_MOVE_DURATION; i++) {
    game::advanceWorldMovementTicks(state, 1);
  }
  ok = assertEqual(state.playerMovementCount, game::TILE_FIELD_BLOOD_MOVE_DURATION,
                   "movement counter") &&
       ok;

  auto* tile = game::tileAtCurrentLayer(state.mapInstances["map_a"], 1, 1);
  ok = assertTrue(tile != nullptr, "tile exists after aging") && ok;
  if (tile) {
    ok = assertEqual(static_cast<int>(tile->fields.size()), 0,
                     "blood expired on unloaded map") &&
         ok;
  }

  if (ok) {
    LOG(INFO) << "TestTileFieldAging PASSED" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << "TestTileFieldAging FAILED" << LOG_ENDL;
  return 1;
}
