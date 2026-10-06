#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "model/instances/ItemInstance.hpp"
#include "model/templates/CharacterTemplate.h"
#include <string_view>

namespace db {
class Database;
}

namespace game {

constexpr std::string_view kGoldBagItemTemplateName = "GoldBag";
constexpr std::string_view kFoodBagItemTemplateName = "FoodBag";

struct DropRollResult {
  enum class Kind { Item, Gold, Food } kind = Kind::Item;
  bmin::String itemName;
  int amount = 0;
};

inline bool isGoldBagItemTemplate(std::string_view templateName) {
  return templateName == kGoldBagItemTemplateName;
}

inline bool isFoodBagItemTemplate(std::string_view templateName) {
  return templateName == kFoodBagItemTemplateName;
}

bmin::String formatMapItemDisplayLabel(const db::Database& database,
                                       const model::ItemInstance& item);

void rollDropTable(std::string_view tableName,
                   const db::Database& database,
                   bmin::DynArray<DropRollResult>& out);

void rollCharacterDropTables(const model::CharacterTemplate& characterTemplate,
                             const db::Database& database,
                             bmin::DynArray<DropRollResult>& out);

void appendDropRollToItems(bmin::DynArray<model::ItemInstance>& items,
                           const DropRollResult& roll,
                           int x,
                           int y);

} // namespace game
