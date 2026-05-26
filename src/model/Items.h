#pragma once

#include "model/StatusEffects.h"
#include <string>
#include <vector>

namespace model {

enum class ItemType {
  WEAPON_MELEE,
  WEAPON_RANGED,
  WEAPON_AMMO,
  GARB,
  PANTS,
  GLOVES,
  HAT,
  SHOES,
  NECKLACE,
  POTION,
  UTILITY,
  UNKNOWN
};

std::string getStringFromItemType(ItemType itemType);
ItemType getItemTypeFromString(const std::string& itemTypeString);

enum class ItemUsability {
  NOT_USABLE,
  USABLE_EVERYWHERE,
  USABLE_TOWN_ONLY,
  USABLE_COMBAT_ONLY,
  USABLE_OUTSIDE_ONLY,
  USABLE_TOWN_AND_COMBAT,
};

enum class ItemUsabilityType { ITEM_USE_NOT_USABLE, ITEM_USE_CAST_SPELL };

struct ItemUsabilityArgs {
  ItemUsabilityType itemUsabilityType = ItemUsabilityType::ITEM_USE_NOT_USABLE;
  std::vector<int> intArgs;
  std::vector<std::string> stringArgs;
};

struct ItemTemplate {
  ItemType itemType = ItemType::UNKNOWN;
  std::string name;
  std::string label;
  std::string iconSpriteName;
  std::string description;
  int weight = 0;
  int value = 0;
  ItemUsability itemUsability = ItemUsability::NOT_USABLE;
  ItemUsabilityArgs itemUsabilityArgs;
  std::vector<model::StatusEffectTemplate> statusEffects;
};

// Represents an item actually in the world
struct ItemInstance {
  std::string id;
  std::string itemName;
  int quantity = 1;
};

} // namespace model
