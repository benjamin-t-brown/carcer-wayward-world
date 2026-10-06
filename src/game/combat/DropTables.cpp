#include "DropTables.h"
#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "model/templates/DropTables.hpp"
#include "model/templates/UtilityTypes.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include <cstdlib>
#include <unordered_set>

namespace game {

namespace {

constexpr int kDropTableMaxDepth = 32;

int effectiveEntryWeight(const model::DropTableEntry& entry) {
  return entry.weight <= 0 ? 1 : entry.weight;
}

const model::DropTableEntry* pickWeightedEntry(
    const bmin::DynArray<model::DropTableEntry>& entries) {
  if (entries.empty()) {
    return nullptr;
  }

  int totalWeight = 0;
  for (const auto& entry : entries) {
    totalWeight += effectiveEntryWeight(entry);
  }
  if (totalWeight <= 0) {
    return &entries[0];
  }

  int roll = std::rand() % totalWeight;
  for (const auto& entry : entries) {
    roll -= effectiveEntryWeight(entry);
    if (roll < 0) {
      return &entry;
    }
  }
  return &entries[entries.size() - 1];
}

int rollInclusiveRange(int min, int max) {
  if (max < min) {
    max = min;
  }
  if (max == min) {
    return min;
  }
  return min + std::rand() % (max - min + 1);
}

void rollDropTableRecursive(std::string_view tableName,
                            const db::Database& database,
                            bmin::DynArray<DropRollResult>& out,
                            std::unordered_set<std::string>& visited,
                            int depth) {
  if (depth >= kDropTableMaxDepth) {
    LOG(WARN) << "rollDropTable: max depth reached at table " << tableName << LOG_ENDL;
    return;
  }

  const bmin::String key(tableName.data(), tableName.size());
  if (visited.contains(key.cStr())) {
    LOG(WARN) << "rollDropTable: cycle detected at table " << tableName << LOG_ENDL;
    return;
  }
  visited.insert(key.cStr());

  const model::DropTableTemplate* table = database.findDropTable(tableName);
  if (table == nullptr) {
    LOG(WARN) << "rollDropTable: unknown drop table " << tableName << LOG_ENDL;
    visited.erase(key.cStr());
    return;
  }

  const model::DropTableEntry* picked = pickWeightedEntry(table->entries);
  if (picked == nullptr) {
    LOG(WARN) << "rollDropTable: empty entries in table " << tableName << LOG_ENDL;
    visited.erase(key.cStr());
    return;
  }

  if (picked->isNothingEntry) {
    visited.erase(key.cStr());
    return;
  }

  if (!picked->dropTable.empty()) {
    rollDropTableRecursive(bmin::toStringView(picked->dropTable), database, out, visited,
                           depth + 1);
    visited.erase(key.cStr());
    return;
  }

  if (!picked->item.empty()) {
    if (database.findItemTemplate(bmin::toStringView(picked->item)) == nullptr) {
      LOG(WARN) << "rollDropTable: unknown item " << picked->item.cStr() << " in table "
                << tableName << LOG_ENDL;
      visited.erase(key.cStr());
      return;
    }

    DropRollResult result;
    result.kind = DropRollResult::Kind::Item;
    result.itemName = picked->item;
    out.pushBack(std::move(result));
    visited.erase(key.cStr());
    return;
  }

  if (picked->isGoldRangeEntry) {
    const int amount = rollInclusiveRange(picked->goldMin, picked->goldMax);
    if (amount > 0) {
      DropRollResult result;
      result.kind = DropRollResult::Kind::Gold;
      result.amount = amount;
      out.pushBack(std::move(result));
    }
    visited.erase(key.cStr());
    return;
  }

  if (picked->isFoodRangeEntry) {
    const int amount = rollInclusiveRange(picked->foodMin, picked->foodMax);
    if (amount > 0) {
      DropRollResult result;
      result.kind = DropRollResult::Kind::Food;
      result.amount = amount;
      out.pushBack(std::move(result));
    }
    visited.erase(key.cStr());
    return;
  }

  LOG(WARN) << "rollDropTable: empty entry in table " << tableName << LOG_ENDL;
  visited.erase(key.cStr());
}

} // namespace

bmin::String formatMapItemDisplayLabel(const db::Database& database,
                                       const model::ItemInstance& item) {
  bmin::String label;
  try {
    const auto& itemTemplate =
        database.getItemTemplate(bmin::toStringView(item.itemTemplateName));
    label = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  } catch (...) {
    label = item.itemTemplateName;
  }

  if (item.amount <= 0) {
    return label;
  }

  if (isGoldBagItemTemplate(bmin::toStringView(item.itemTemplateName))) {
    return label + " (" + bmin::toString(item.amount) + TRANSLATE(" gp") + ")";
  }
  if (isFoodBagItemTemplate(bmin::toStringView(item.itemTemplateName))) {
    return label + " (" + bmin::toString(item.amount) + " " + TRANSLATE("food") + ")";
  }
  return label;
}

void rollDropTable(std::string_view tableName,
                   const db::Database& database,
                   bmin::DynArray<DropRollResult>& out) {
  if (tableName.empty()) {
    return;
  }
  std::unordered_set<std::string> visited;
  rollDropTableRecursive(tableName, database, out, visited, 0);
}

void rollCharacterDropTables(const model::CharacterTemplate& characterTemplate,
                             const db::Database& database,
                             bmin::DynArray<DropRollResult>& out) {
  for (const auto& tableName : characterTemplate.combat.dropTables) {
    if (tableName.empty()) {
      continue;
    }
    rollDropTable(bmin::toStringView(tableName), database, out);
  }
}

void appendDropRollToItems(bmin::DynArray<model::ItemInstance>& items,
                           const DropRollResult& roll,
                           int x,
                           int y) {
  model::ItemInstance dropped;
  dropped.id = model::createRandomId();
  dropped.quantity = 1;
  dropped.x = x;
  dropped.y = y;

  switch (roll.kind) {
  case DropRollResult::Kind::Item:
    dropped.itemTemplateName = roll.itemName;
    break;
  case DropRollResult::Kind::Gold:
    if (roll.amount <= 0) {
      return;
    }
    dropped.itemTemplateName = bmin::String(kGoldBagItemTemplateName.data(),
                                            kGoldBagItemTemplateName.size());
    dropped.amount = roll.amount;
    break;
  case DropRollResult::Kind::Food:
    if (roll.amount <= 0) {
      return;
    }
    dropped.itemTemplateName = bmin::String(kFoodBagItemTemplateName.data(),
                                            kFoodBagItemTemplateName.size());
    dropped.amount = roll.amount;
    break;
  }

  items.pushBack(std::move(dropped));
}

} // namespace game
