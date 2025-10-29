#pragma once

#include "types/StatusEffects.h"
#include <string>
#include <vector>

namespace types {

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

inline std::string getStringFromItemType(ItemType itemType) {
  switch (itemType) {
  case ItemType::WEAPON_MELEE:
    return "Melee Weapon";
  case ItemType::WEAPON_RANGED:
    return "Ranged Weapon";
  case ItemType::WEAPON_AMMO:
    return "Ammo";
  case ItemType::GARB:
    return "Garb";
  case ItemType::PANTS:
    return "Pants";
  case ItemType::GLOVES:
    return "Gloves";
  case ItemType::HAT:
    return "Hat";
  case ItemType::SHOES:
    return "Shoes";
  case ItemType::NECKLACE:
    return "Necklace";
  case ItemType::POTION:
    return "Potion";
  case ItemType::UTILITY:
    return "Utility";
  default:
    return "Unknown";
  }
  return "Unknown";
}

inline ItemType getItemTypeFromString(const std::string& itemTypeString) {
  if (itemTypeString == "Melee Weapon") {
    return ItemType::WEAPON_MELEE;
  } else if (itemTypeString == "Ranged Weapon") {
    return ItemType::WEAPON_RANGED;
  } else if (itemTypeString == "Ammo") {
    return ItemType::WEAPON_AMMO;
  } else if (itemTypeString == "Garb") {
    return ItemType::GARB;
  } else if (itemTypeString == "Pants") {
    return ItemType::PANTS;
  } else if (itemTypeString == "Gloves") {
    return ItemType::GLOVES;
  } else if (itemTypeString == "Hat") {
    return ItemType::HAT;
  } else if (itemTypeString == "Shoes") {
    return ItemType::SHOES;
  } else if (itemTypeString == "Necklace") {
    return ItemType::NECKLACE;
  } else if (itemTypeString == "Potion") {
    return ItemType::POTION;
  } else if (itemTypeString == "Utility") {
    return ItemType::UTILITY;
  }
  return ItemType::UNKNOWN;
}

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
  std::vector<types::StatusEffectTemplate> statusEffects;
};

} // namespace types