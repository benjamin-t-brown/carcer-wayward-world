#include "db/Database.h"
#include "db/loaders/LoadMapGridTemplates.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"
#include "bmin/Map.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual, const char* expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected '" << expected << "' but got '" << actual.cStr() << "'"
               << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  LOG(INFO) << "Starting TestLoadMapGridTemplates" << LOG_ENDL;

  try {
    bmin::Map<bmin::String, model::MapGridTemplate> grids;
    db::loadMapGridTemplates("__test__/assets/map-grids-fixture.json", grids);

    const auto it = grids.find(bmin::String("test_grid"));
    if (it == grids.end()) {
      LOG(ERROR) << "Missing test_grid" << LOG_ENDL;
      return 1;
    }

    const model::MapGridTemplate& grid = it->value;
    bool ok = true;
    ok = assertEqual(grid.gridWidth, 2, "test_grid.gridWidth") && ok;
    ok = assertEqual(grid.gridHeight, 2, "test_grid.gridHeight") && ok;
    ok = assertEqual(grid.mapWidth, 10, "test_grid.mapWidth") && ok;
    ok = assertEqual(grid.mapHeight, 12, "test_grid.mapHeight") && ok;
    ok = assertEqual(static_cast<int>(grid.cells.size()), 2, "test_grid.cells.rows") && ok;
    ok = assertEqual(static_cast<int>(grid.cells[0].size()), 2, "test_grid.cells[0].cols") && ok;
    ok = assertEqualStr(grid.cells[0][0], "map_a", "test_grid.cells[0][0]") && ok;
    ok = assertEqualStr(grid.cells[0][1], "map_b", "test_grid.cells[0][1]") && ok;
    ok = assertEqualStr(grid.cells[1][0], "", "test_grid.cells[1][0]") && ok;
    ok = assertEqualStr(grid.cells[1][1], "map_c", "test_grid.cells[1][1]") && ok;
    ok = assertEqualStr(grid.label, "Test Grid", "test_grid.label") && ok;

    db::Database database;
    database.load();
    const model::MapGridTemplate& loaded = database.getMapGridTemplate("OutsideAlinea");
    ok = assertEqual(loaded.gridWidth, 3, "OutsideAlinea.gridWidth") && ok;
    ok = assertEqual(loaded.gridHeight, 2, "OutsideAlinea.gridHeight") && ok;
    ok = assertEqual(loaded.mapWidth, 30, "OutsideAlinea.mapWidth") && ok;
    ok = assertEqual(loaded.mapHeight, 30, "OutsideAlinea.mapHeight") && ok;
    ok = assertEqualStr(loaded.cells[0][0], "alinea_outsideAlinea2", "OutsideAlinea.cells[0][0]") &&
         ok;

    if (!ok) {
      LOG(ERROR) << "Map grid template assertions failed" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadMapGridTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading map grid templates: " << e.what() << LOG_ENDL;
    return 1;
  }
}
