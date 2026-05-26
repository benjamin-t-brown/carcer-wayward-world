#include "model/Items.h"

namespace model {

std::string getStringFromItemType(ItemType itemType) {
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

ItemType getItemTypeFromString(const std::string& itemTypeString) {
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

} // namespace model
