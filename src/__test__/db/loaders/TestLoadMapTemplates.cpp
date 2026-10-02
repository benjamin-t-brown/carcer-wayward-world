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
    const bmin::DynArray<int>& layer0 = map.tiles[0];
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
