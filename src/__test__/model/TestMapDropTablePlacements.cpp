#include "db/Database.h"
#include "game/map/MapPersistence.h"
#include "model/templates/DropTables.hpp"
#include "model/templates/Items.h"
#include "model/instances/MapInstance.h"
#include "model/templates/Maps.h"
#include "sdl2w/Logger.h"

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

model::CarcerMapTemplate makeDropMapTemplate() {
  auto mapTemplate = model::CarcerMapTemplate{};
  mapTemplate.name = "drop_table_map";
  mapTemplate.label = "Drop Table Map";
  mapTemplate.width = 2;
  mapTemplate.height = 2;
  mapTemplate.tilesets.pushBack("terrain0");

  auto layer = bmin::DynArray<int>{};
  for (auto i = 0; i < 4; i++) {
    layer.pushBack(0);
    layer.pushBack(0);
  }
  mapTemplate.tiles[0] = std::move(layer);

  auto placement = model::MapItemPlacement{};
  placement.l = 0;
  placement.i = 1;
  placement.dropTable = "mapOnlyDagger";
  mapTemplate.items.pushBack(std::move(placement));
  return mapTemplate;
}

void addDaggerDropTable(db::Database& database) {
  model::ItemTemplate dagger;
  dagger.name = "Dagger";
  dagger.weight = 1;
  database.addItemTemplate(dagger);

  model::DropTableEntry onlyEntry;
  onlyEntry.item = "Dagger";
  model::DropTableTemplate table;
  table.name = "mapOnlyDagger";
  table.label = "Map Only Dagger";
  table.entries.pushBack(onlyEntry);
  database.addDropTable(table);
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestMapDropTablePlacements" << LOG_ENDL;

  db::Database database;
  addDaggerDropTable(database);
  database.addMapTemplate(makeDropMapTemplate());

  auto instances = game::createMapInstances(database);
  const auto it = instances.find(bmin::String("drop_table_map"));
  if (it == instances.end()) {
    LOG(ERROR) << "Missing drop_table_map instance" << LOG_ENDL;
    return 1;
  }

  const model::MapInstance& map = it->value;
  bool ok = true;
  ok = assertEqual(static_cast<int>(map.persistentState.items.size()), 1,
                   "drop table placement rolls one item") &&
       ok;
  if (!map.persistentState.items.empty()) {
    const auto& item = map.persistentState.items[0];
    ok = assertTrue(item.itemTemplateName == "Dagger", "rolled item template") && ok;
    ok = assertEqual(item.x, 1, "rolled item x") && ok;
    ok = assertEqual(item.y, 0, "rolled item y") && ok;
  }

  const int itemCountAfterFirstCreate = static_cast<int>(map.persistentState.items.size());
  auto fromTemplateOnly = model::createMapInstanceFromTemplate(makeDropMapTemplate());
  ok = assertEqual(static_cast<int>(fromTemplateOnly.persistentState.items.size()), 0,
                   "createMapInstanceFromTemplate does not roll drop tables") &&
       ok;
  ok = assertEqual(itemCountAfterFirstCreate, 1,
                   "document: only createMapInstances materializes drop placements") &&
       ok;

  if (!ok) {
    LOG(ERROR) << "TestMapDropTablePlacements FAILED" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestMapDropTablePlacements completed successfully" << LOG_ENDL;
  return 0;
}
