#include "db/Database.h"
#include "db/loaders/LoadMapTemplates.h"
#include "model/instances/MapInstance.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/Map.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool tileHasNoLock(const model::MapInstance& map, int x, int y) {
  const auto* tile = model::mapInstanceGetTileAt(map, x, y, 0);
  return tile != nullptr && !tile->doorLock.has_value();
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadMapTemplates" << LOG_ENDL;

  try {
    bmin::Map<bmin::String, model::CarcerMapTemplate> maps;
    db::loadMapTemplates("__test__/assets/maps-flat-fixture.json", maps);

    const auto it = maps.find(bmin::String("flat_test_map"));
    if (it == maps.end()) {
      LOG(ERROR) << "Missing flat_test_map" << LOG_ENDL;
      return 1;
    }

    const model::CarcerMapTemplate& map = it->value;
    bool ok = true;
    ok = assertEqual(map.width, 2, "flat_test_map.width") && ok;
    ok = assertEqual(map.height, 2, "flat_test_map.height") && ok;
    ok = assertEqual(static_cast<int>(map.tilesets.size()), 2, "flat_test_map.tilesets") && ok;
    const auto layer0It = map.tiles.find(0);
    if (layer0It == map.tiles.end()) {
      LOG(ERROR) << "flat_test_map missing layer 0 tiles" << LOG_ENDL;
      return 1;
    }
    const bmin::DynArray<int>& layer0 = layer0It->value;
    ok = assertEqual(static_cast<int>(layer0.size()), 8, "flat_test_map.tiles[0].size") && ok;
    ok = assertEqual(layer0[2], 1, "flat_test_map.tiles[0][2]") && ok;
    ok = assertEqual(layer0[3], 6, "flat_test_map.tiles[0][3]") && ok;
    ok = assertEqual(static_cast<int>(map.characters.size()), 2, "flat_test_map.characters") && ok;
    ok = assertEqual(map.characters[0].l, 0, "flat_test_map.characters[0].l") && ok;
    ok = assertEqual(map.characters[0].i, 1, "flat_test_map.characters[0].i") && ok;
    ok = assertEqual(map.characters[0].flipped ? 1 : 0, 0, "flat_test_map.characters[0].flipped") &&
         ok;
    ok = assertEqual(map.characters[1].flipped ? 1 : 0, 1, "flat_test_map.characters[1].flipped") &&
         ok;
    ok = assertEqual(static_cast<int>(map.items.size()), 1, "flat_test_map.items") && ok;
    ok = assertEqual(map.items[0].quantity, 2, "flat_test_map.items[0].quantity") && ok;
    ok = assertEqual(static_cast<int>(map.markers.size()), 1, "flat_test_map.markers") && ok;
    ok = assertEqual(static_cast<int>(map.eventTriggers.size()), 1, "flat_test_map.eventTriggers") && ok;
    if (!map.eventTriggers.empty()) {
      ok = assertTrue(map.eventTriggers[0].condition == "IS(opened)",
                      "flat_test_map.eventTriggers[0].condition") &&
           ok;
    }
    ok = assertEqual(static_cast<int>(map.travelTriggers.size()), 1, "flat_test_map.travelTriggers") && ok;
    ok = assertEqual(static_cast<int>(map.doorLocks.size()), 0, "flat_test_map.doorLocks") && ok;
    {
      auto flatInstance = model::createMapInstanceFromTemplate(map);
      ok = assertTrue(tileHasNoLock(flatInstance, 0, 0), "flat_test_map tile 0,0 has no lock") && ok;
      ok = assertTrue(tileHasNoLock(flatInstance, 1, 0), "flat_test_map tile 1,0 has no lock") && ok;
      ok = assertTrue(tileHasNoLock(flatInstance, 0, 1), "flat_test_map tile 0,1 has no lock") && ok;
      ok = assertTrue(tileHasNoLock(flatInstance, 1, 1), "flat_test_map tile 1,1 has no lock") && ok;
    }

    bmin::Map<bmin::String, model::CarcerMapTemplate> doorLockMaps;
    db::loadMapTemplates("__test__/db/loaders/door-locks-fixture.json", doorLockMaps);

    const auto lockIt = doorLockMaps.find(bmin::String("door_lock_map"));
    if (lockIt == doorLockMaps.end()) {
      LOG(ERROR) << "Missing door_lock_map" << LOG_ENDL;
      return 1;
    }
    const model::CarcerMapTemplate& locked = lockIt->value;
    ok = assertEqual(static_cast<int>(locked.doorLocks.size()), 2, "door_lock_map.doorLocks") && ok;
    if (locked.doorLocks.size() >= 2) {
      ok = assertEqual(locked.doorLocks[0].l, 0, "door_lock_map.doorLocks[0].l") && ok;
      ok = assertEqual(locked.doorLocks[0].i, 0, "door_lock_map.doorLocks[0].i") && ok;
      ok = assertEqual(locked.doorLocks[0].lockLevel, 0, "door_lock_map.doorLocks[0].lockLevel") &&
           ok;
      ok = assertTrue(locked.doorLocks[0].keyItem == "que_realmShedKey",
                      "door_lock_map.doorLocks[0].keyItem") &&
           ok;
      ok = assertEqual(locked.doorLocks[1].l, 0, "door_lock_map.doorLocks[1].l") && ok;
      ok = assertEqual(locked.doorLocks[1].i, 1, "door_lock_map.doorLocks[1].i") && ok;
      ok = assertEqual(locked.doorLocks[1].lockLevel, 12, "door_lock_map.doorLocks[1].lockLevel") &&
           ok;
      ok = assertTrue(locked.doorLocks[1].keyItem == "",
                      "door_lock_map.doorLocks[1].keyItem dropped") &&
           ok;
    }
    {
      auto lockedInstance = model::createMapInstanceFromTemplate(locked);
      const auto* keyTile = model::mapInstanceGetTileAt(lockedInstance, 0, 0, 0);
      const auto* levelTile = model::mapInstanceGetTileAt(lockedInstance, 1, 0, 0);
      ok = assertTrue(keyTile != nullptr && keyTile->doorLock.has_value(),
                      "door_lock_map tile 0 has lock") &&
           ok;
      ok = assertTrue(levelTile != nullptr && levelTile->doorLock.has_value(),
                      "door_lock_map tile 1 has lock") &&
           ok;
      if (keyTile && keyTile->doorLock) {
        ok = assertEqual(keyTile->doorLock->lockLevel, 0, "door_lock_map tile 0 lockLevel") && ok;
        ok = assertTrue(keyTile->doorLock->keyItem == "que_realmShedKey",
                        "door_lock_map tile 0 keyItem") &&
             ok;
      }
      if (levelTile && levelTile->doorLock) {
        ok = assertEqual(levelTile->doorLock->lockLevel, 12, "door_lock_map tile 1 lockLevel") &&
             ok;
        ok = assertTrue(levelTile->doorLock->keyItem == "",
                        "door_lock_map tile 1 keyItem empty") &&
             ok;
      }
    }

    const auto unlockedIt = doorLockMaps.find(bmin::String("no_door_lock_map"));
    if (unlockedIt == doorLockMaps.end()) {
      LOG(ERROR) << "Missing no_door_lock_map" << LOG_ENDL;
      return 1;
    }
    const model::CarcerMapTemplate& unlocked = unlockedIt->value;
    ok = assertEqual(static_cast<int>(unlocked.doorLocks.size()), 0, "no_door_lock_map.doorLocks") &&
         ok;
    {
      auto unlockedInstance = model::createMapInstanceFromTemplate(unlocked);
      ok = assertTrue(tileHasNoLock(unlockedInstance, 0, 0), "no_door_lock_map tile 0 has no lock") &&
           ok;
      ok = assertTrue(tileHasNoLock(unlockedInstance, 1, 0), "no_door_lock_map tile 1 has no lock") &&
           ok;
    }

    db::Database database;
    database.load();
    const model::CarcerMapTemplate& loaded = database.getMapTemplate("alinea_outside1");
    ok = assertEqual(loaded.width, 30, "alinea_outside1.width") && ok;
    ok = assertEqual(static_cast<int>(loaded.layers.size()), 2, "alinea_outside1.layers") && ok;

    const model::CarcerMapTemplate& alinea2 =
        database.getMapTemplate("alinea_outsideAlinea2");
    ok = assertTrue(alinea2.tiles.contains(-1), "alinea_outsideAlinea2.tiles[-1] loaded") && ok;
    ok = assertTrue(alinea2.tiles.contains(0), "alinea_outsideAlinea2.tiles[0] loaded") && ok;
    ok = assertTrue(alinea2.tiles.contains(1), "alinea_outsideAlinea2.tiles[1] loaded") && ok;
    {
      auto alinea2Instance = model::createMapInstanceFromTemplate(alinea2);
      ok = assertTrue(model::mapHasLayer(model::mapInstanceTiles(alinea2Instance), -1),
                      "alinea_outsideAlinea2 instance layer -1") &&
           ok;
      constexpr int basementIndex = 662;
      constexpr int mapWidth = 30;
      const int basementX = basementIndex % mapWidth;
      const int basementY = basementIndex / mapWidth;
      const auto* basementTile =
          model::mapInstanceGetTileAt(alinea2Instance, basementX, basementY, -1);
      ok = assertTrue(basementTile != nullptr, "alinea_outsideAlinea2 basement tile exists") && ok;
      if (basementTile) {
        ok = assertTrue(!basementTile->tilesetName.empty(),
                        "alinea_outsideAlinea2 basement tileset non-empty") &&
             ok;
        ok = assertTrue(basementTile->travelTrigger.has_value(),
                        "alinea_outsideAlinea2 basement travel trigger") &&
             ok;
        if (basementTile->travelTrigger) {
          ok = assertTrue(basementTile->travelTrigger->requiresAction,
                          "alinea_outsideAlinea2 basement travel requiresAction") &&
               ok;
          ok = assertEqual(basementTile->travelTrigger->destinationLayer, 0,
                           "alinea_outsideAlinea2 basement travel destinationLayer") &&
               ok;
        }
      }
    }

    if (!ok) {
      LOG(ERROR) << "Map template assertions failed" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadMapTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading map templates: " << e.what() << LOG_ENDL;
    return 1;
  }
}
