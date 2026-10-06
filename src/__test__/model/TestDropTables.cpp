#include "db/Database.h"
#include "game/combat/DropTables.h"
#include "model/templates/DropTables.hpp"
#include "model/templates/Items.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"
#include <cstdlib>

namespace {

void addItem(db::Database& database, const bmin::String& name) {
  model::ItemTemplate item;
  item.name = name;
  item.weight = 1;
  database.addItemTemplate(item);
}

model::DropTableTemplate makeTable(const bmin::String& name,
                                   const bmin::DynArray<model::DropTableEntry>& entries) {
  model::DropTableTemplate table;
  table.name = name;
  table.label = name;
  table.entries = entries;
  return table;
}

bool assertEqualSize(size_t actual, size_t expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected size " << expected << " got " << actual << LOG_ENDL;
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

bool assertEqualStr(const bmin::String& actual, const bmin::String& expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected '" << expected.cStr() << "' got '" << actual.cStr() << "'"
               << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualInt(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestDropTables" << LOG_ENDL;
  bool ok = true;

  db::Database database;
  addItem(database, "PotionHealing");
  addItem(database, "DaggerBronze");
  addItem(database, "ArrowsIron");

  {
    model::DropTableEntry leafEntry;
    leafEntry.item = "PotionHealing";
    model::DropTableEntry nestedEntry;
    nestedEntry.dropTable = "fixtureLeaf";
    database.addDropTable(makeTable("fixtureLeaf", {leafEntry}));
    database.addDropTable(makeTable("fixtureNested", {nestedEntry}));

    bmin::DynArray<game::DropRollResult> rolled;
    game::rollDropTable("fixtureNested", database, rolled);
    ok = assertEqualSize(rolled.size(), 1, "nested roll size") && ok;
    if (!rolled.empty()) {
      ok = assertTrue(rolled[0].kind == game::DropRollResult::Kind::Item, "nested roll kind") &&
           ok;
      ok = assertEqualStr(rolled[0].itemName, "PotionHealing", "nested roll item") && ok;
    }
  }

  {
    model::DropTableEntry onlyEntry;
    onlyEntry.item = "DaggerBronze";
    database.addDropTable(makeTable("onlyDagger", {onlyEntry}));

    std::srand(1);
    bmin::DynArray<game::DropRollResult> rolled;
    game::rollDropTable("onlyDagger", database, rolled);
    ok = assertEqualSize(rolled.size(), 1, "single entry roll size") && ok;
    if (!rolled.empty()) {
      ok = assertEqualStr(rolled[0].itemName, "DaggerBronze", "single entry item") && ok;
    }
  }

  {
    model::DropTableEntry entryA;
    entryA.item = "PotionHealing";
    entryA.weight = 1;
    model::DropTableEntry entryB;
    entryB.item = "ArrowsIron";
    entryB.weight = 1;
    database.addDropTable(makeTable("coinFlip", {entryA, entryB}));

    std::srand(42);
    bmin::DynArray<game::DropRollResult> rolled;
    game::rollDropTable("coinFlip", database, rolled);

    std::srand(42);
    bmin::DynArray<game::DropRollResult> rolledAgain;
    game::rollDropTable("coinFlip", database, rolledAgain);

    ok = assertEqualSize(rolled.size(), 1, "weighted roll size") && ok;
    ok = assertEqualSize(rolledAgain.size(), 1, "weighted roll repeat size") && ok;
    if (!rolled.empty() && !rolledAgain.empty()) {
      ok = assertEqualStr(rolled[0].itemName, rolledAgain[0].itemName,
                          "weighted roll deterministic") &&
           ok;
      ok = assertTrue(rolled[0].itemName == "PotionHealing" || rolled[0].itemName == "ArrowsIron",
                      "weighted roll known item") &&
           ok;
    }
  }

  {
    model::DropTableEntry goldEntry;
    goldEntry.isGoldRangeEntry = true;
    goldEntry.goldMin = 5;
    goldEntry.goldMax = 8;
    database.addDropTable(makeTable("goldRange", {goldEntry}));

    std::srand(99);
    bmin::DynArray<game::DropRollResult> rolled;
    game::rollDropTable("goldRange", database, rolled);

    std::srand(99);
    bmin::DynArray<game::DropRollResult> rolledAgain;
    game::rollDropTable("goldRange", database, rolledAgain);

    ok = assertEqualSize(rolled.size(), 1, "gold roll size") && ok;
    if (!rolled.empty()) {
      ok = assertTrue(rolled[0].kind == game::DropRollResult::Kind::Gold, "gold roll kind") &&
           ok;
      ok = assertTrue(rolled[0].amount >= 5 && rolled[0].amount <= 8, "gold roll in range") &&
           ok;
    }
    if (!rolled.empty() && !rolledAgain.empty()) {
      ok = assertEqualInt(rolled[0].amount, rolledAgain[0].amount, "gold roll deterministic") &&
           ok;
    }
  }

  {
    model::DropTableEntry nothingEntry;
    nothingEntry.isNothingEntry = true;
    database.addDropTable(makeTable("nothingOnly", {nothingEntry}));

    bmin::DynArray<game::DropRollResult> rolled;
    game::rollDropTable("nothingOnly", database, rolled);
    ok = assertEqualSize(rolled.size(), 0, "nothing roll produces no results") && ok;
  }

  {
    model::DropTableEntry zeroGoldEntry;
    zeroGoldEntry.isGoldRangeEntry = true;
    zeroGoldEntry.goldMin = 0;
    zeroGoldEntry.goldMax = 0;
    database.addDropTable(makeTable("zeroGold", {zeroGoldEntry}));

    bmin::DynArray<game::DropRollResult> rolled;
    game::rollDropTable("zeroGold", database, rolled);
    ok = assertEqualSize(rolled.size(), 0, "zero gold roll produces nothing") && ok;
  }

  {
    model::DropTableEntry zeroFoodEntry;
    zeroFoodEntry.isFoodRangeEntry = true;
    zeroFoodEntry.foodMin = 0;
    zeroFoodEntry.foodMax = 0;
    database.addDropTable(makeTable("zeroFood", {zeroFoodEntry}));

    bmin::DynArray<game::DropRollResult> rolled;
    game::rollDropTable("zeroFood", database, rolled);
    ok = assertEqualSize(rolled.size(), 0, "zero food roll produces nothing") && ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestDropTables assertions failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestDropTables completed successfully" << LOG_ENDL;
  return 0;
}
