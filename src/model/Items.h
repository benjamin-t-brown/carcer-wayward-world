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

inline std::string getStringFromItemType(ItemType itemType) {
  switch (itemType) {
  case ItemType::WEAPON_MELEE:
    return "WEAPON_MELEE";
  case ItemType::WEAPON_RANGED:
    return "WEAPON_RANGED";
  case ItemType::WEAPON_AMMO:
    return "WEAPON_AMMO";
  case ItemType::GARB:
    return "GARB";
  case ItemType::PANTS:
    return "PANTS";
  case ItemType::GLOVES:
    return "GLOVES";
  case ItemType::HAT:
    return "HAT";
  case ItemType::SHOES:
    return "SHOES";
  case ItemType::NECKLACE:
    return "NECKLACE";
  case ItemType::POTION:
    return "POTION";
  case ItemType::UTILITY:
    return "UTILITY";
  default:
    return "UNKNOWN";
  }
  return "UNKNOWN";
}

inline ItemType getItemTypeFromString(const std::string& itemTypeString) {
  if (itemTypeString == "WEAPON_MELEE") {
    return ItemType::WEAPON_MELEE;
  } else if (itemTypeString == "WEAPON_RANGED") {
    return ItemType::WEAPON_RANGED;
  } else if (itemTypeString == "WEAPON_AMMO") {
    return ItemType::WEAPON_AMMO;
  } else if (itemTypeString == "GARB") {
    return ItemType::GARB;
  } else if (itemTypeString == "PANTS") {
    return ItemType::PANTS;
  } else if (itemTypeString == "GLOVES") {
    return ItemType::GLOVES;
  } else if (itemTypeString == "HAT") {
    return ItemType::HAT;
  } else if (itemTypeString == "SHOES") {
    return ItemType::SHOES;
  } else if (itemTypeString == "NECKLACE") {
    return ItemType::NECKLACE;
  } else if (itemTypeString == "POTION") {
    return ItemType::POTION;
  } else if (itemTypeString == "UTILITY") {
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
  std::vector<model::StatusEffectTemplate> statusEffects;
};

} // namespace model