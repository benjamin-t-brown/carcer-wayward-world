#include "db/loaders/LoadDropTables.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"
#include "bmin/Map.h"

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  LOG(INFO) << "Starting TestLoadDropTables" << LOG_ENDL;

  bmin::Map<bmin::String, model::DropTableTemplate> dropTables;

  try {
    db::loadDropTables("assets/db/drop-tables.json", dropTables);
    if (dropTables.size() < 2) {
      LOG(ERROR) << "Expected at least 2 drop tables from assets, got " << dropTables.size()
                 << LOG_ENDL;
      return 1;
    }

    const auto easyIt = dropTables.find(bmin::String("dropsEasyMisc"));
    if (easyIt == dropTables.end()) {
      LOG(ERROR) << "Missing dropsEasyMisc" << LOG_ENDL;
      return 1;
    }
    if (easyIt->value.entries.size() != 2) {
      LOG(ERROR) << "dropsEasyMisc expected 2 entries" << LOG_ENDL;
      return 1;
    }
    if (easyIt->value.entries[0].item != bmin::String("PotionHealing") ||
        easyIt->value.entries[0].weight != 2) {
      LOG(ERROR) << "dropsEasyMisc first entry mismatch" << LOG_ENDL;
      return 1;
    }

    bmin::Map<bmin::String, model::DropTableTemplate> fixtures;
    db::loadDropTables("__test__/db/loaders/drop-tables-fixture.json", fixtures);
    const auto nestedIt = fixtures.find(bmin::String("fixtureNested"));
    if (nestedIt == fixtures.end()) {
      LOG(ERROR) << "Missing fixtureNested" << LOG_ENDL;
      return 1;
    }
    if (nestedIt->value.entries[0].dropTable != bmin::String("fixtureLeaf")) {
      LOG(ERROR) << "fixtureNested dropTable reference mismatch" << LOG_ENDL;
      return 1;
    }

    const auto weightedIt = fixtures.find(bmin::String("fixtureWeighted"));
    if (weightedIt == fixtures.end() ||
        weightedIt->value.entries[1].weight != 1) {
      LOG(ERROR) << "fixtureWeighted non-positive weight should normalize to 1" << LOG_ENDL;
      return 1;
    }

    const auto goldIt = fixtures.find(bmin::String("fixtureGoldRange"));
    if (goldIt == fixtures.end()) {
      LOG(ERROR) << "Missing fixtureGoldRange" << LOG_ENDL;
      return 1;
    }
    if (!goldIt->value.entries[0].isGoldRangeEntry ||
        goldIt->value.entries[0].goldMin != 5 || goldIt->value.entries[0].goldMax != 20) {
      LOG(ERROR) << "fixtureGoldRange entry mismatch" << LOG_ENDL;
      return 1;
    }

    const auto foodIt = fixtures.find(bmin::String("fixtureFoodRange"));
    if (foodIt == fixtures.end() || !foodIt->value.entries[0].isFoodRangeEntry ||
        foodIt->value.entries[0].foodMin != 1 || foodIt->value.entries[0].foodMax != 3) {
      LOG(ERROR) << "fixtureFoodRange entry mismatch" << LOG_ENDL;
      return 1;
    }

    const auto nothingIt = fixtures.find(bmin::String("fixtureNothing"));
    if (nothingIt == fixtures.end() || !nothingIt->value.entries[0].isNothingEntry) {
      LOG(ERROR) << "fixtureNothing entry mismatch" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadDropTables completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "TestLoadDropTables failed: " << e.what() << LOG_ENDL;
    return 1;
  }
}
