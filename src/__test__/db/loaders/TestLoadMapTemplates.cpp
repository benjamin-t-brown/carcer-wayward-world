#include "db/Database.h"
#include "db/loaders/LoadMapTemplates.h"
#include "lib/sdl2w/Logger.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadMapTemplates" << LOG_ENDL;

  try {
    std::unordered_map<std::string, model::CarcerMapTemplate> maps;
    db::loadMapTemplates("__test__/assets/maps-flat-fixture.json", maps);

    const auto it = maps.find("flat_test_map");
    if (it == maps.end()) {
      LOG(ERROR) << "Missing flat_test_map" << LOG_ENDL;
      return 1;
    }

    const model::CarcerMapTemplate& map = it->second;
    bool ok = true;
    ok = assertEqual(map.width, 2, "flat_test_map.width") && ok;
    ok = assertEqual(map.height, 2, "flat_test_map.height") && ok;
    ok = assertEqual(static_cast<int>(map.tilesets.size()), 2, "flat_test_map.tilesets") && ok;
    ok = assertEqual(map.tiles.at(0).size(), 8, "flat_test_map.tiles[0].size") && ok;
    ok = assertEqual(map.tiles.at(0)[2], 1, "flat_test_map.tiles[0][2]") && ok;
    ok = assertEqual(map.tiles.at(0)[3], 6, "flat_test_map.tiles[0][3]") && ok;
    ok = assertEqual(static_cast<int>(map.characters.size()), 1, "flat_test_map.characters") && ok;
    ok = assertEqual(map.characters[0].l, 0, "flat_test_map.characters[0].l") && ok;
    ok = assertEqual(map.characters[0].i, 1, "flat_test_map.characters[0].i") && ok;
    ok = assertEqual(static_cast<int>(map.items.size()), 1, "flat_test_map.items") && ok;
    ok = assertEqual(map.items[0].quantity, 2, "flat_test_map.items[0].quantity") && ok;
    ok = assertEqual(static_cast<int>(map.markers.size()), 1, "flat_test_map.markers") && ok;
    ok = assertEqual(static_cast<int>(map.eventTriggers.size()), 1, "flat_test_map.eventTriggers") && ok;
    ok = assertEqual(static_cast<int>(map.travelTriggers.size()), 1, "flat_test_map.travelTriggers") && ok;

    db::Database database;
    database.load();
    const model::CarcerMapTemplate& loaded = database.getMapTemplate("alinea_outside1");
    ok = assertEqual(loaded.width, 30, "alinea_outside1.width") && ok;
    ok = assertEqual(static_cast<int>(loaded.layers.size()), 1, "alinea_outside1.layers") && ok;

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
