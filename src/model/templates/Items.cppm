module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>

export module carcer.model.templates.Items;
export import bmin.containers;
import bmin.string_interop;
export import carcer.model.templates.AbilityTypes;
export import carcer.model.templates.RuneTypes;
import sdl2w;
#include "macros.h"

export {

// --- from model/templates/Items.h ---
namespace model {

enum class ItemType {
  WEAPON_MELEE,
  WEAPON_MELEE_2H,
  WEAPON_RANGED,
  WEAPON_AMMO,
  SHIELD,
  GARB,
  PANTS,
  GLOVES,
  HAT,
  SHOES,
  NECKLACE,
  POTION,
  UTILITY,
  RUNE,
  UNKNOWN
};

bmin::String getStringFromItemType(ItemType itemType);
ItemType getItemTypeFromString(const bmin::String& itemTypeString);

bool itemTypeIsEquippable(ItemType itemType);
bool itemTypeIsTwoHandedWeapon(ItemType itemType);
bool itemTypeUsesWeaponSlots(ItemType itemType);
bool itemTypeUsesRuneSlots(ItemType itemType);

enum class ItemUsability {
  NOT_USABLE,
  USABLE_EVERYWHERE,
  USABLE_TOWN_ONLY,
  USABLE_COMBAT_ONLY,
  USABLE_OUTSIDE_ONLY,
  USABLE_TOWN_AND_COMBAT,
};

ItemUsability getItemUsabilityFromString(const bmin::String& value);

struct ItemWeaponConfig {
  bmin::String abilityName;
  bmin::DynArray<AbilityAttackDmg> dmgOverrides;
};

struct ItemUseAbilityConfig {
  bmin::String abilityName;
  bmin::DynArray<AbilityAttackDmg> dmgOverrides;
  bmin::DynArray<AbilityRestore> restoreOverrides;
};

struct ItemTemplate {
  ItemType itemType = ItemType::UNKNOWN;
  bmin::String name;
  bmin::String label;
  bmin::String iconSpriteName;
  bmin::String description;
  int weight = 0;
  int value = 0;
  bool stackable = false;
  bool indestructable = false;
  ItemUsability itemUsability = ItemUsability::NOT_USABLE;
  std::optional<ItemUseAbilityConfig> useAbility;
  std::optional<bmin::String> useSpecialEvent;
  bmin::DynArray<bmin::String> statusEffectNames;
  std::optional<ItemWeaponConfig> weapon;
  std::optional<RuneType> runeType;
};

} // namespace model

} // export

namespace model {

bmin::String getStringFromItemType(ItemType itemType) {
  switch (itemType) {
  case ItemType::WEAPON_MELEE:
    return "WEAPON_MELEE";
  case ItemType::WEAPON_MELEE_2H:
    return "WEAPON_MELEE_2H";
  case ItemType::WEAPON_RANGED:
    return "WEAPON_RANGED";
  case ItemType::WEAPON_AMMO:
    return "WEAPON_AMMO";
  case ItemType::SHIELD:
    return "SHIELD";
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
  case ItemType::RUNE:
    return "RUNE";
  default:
    return "UNKNOWN";
  }
  return "UNKNOWN";
}

ItemType getItemTypeFromString(const bmin::String& itemTypeString) {
  if (itemTypeString == "WEAPON_MELEE") {
    return ItemType::WEAPON_MELEE;
  } else if (itemTypeString == "WEAPON_MELEE_2H") {
    return ItemType::WEAPON_MELEE_2H;
  } else if (itemTypeString == "WEAPON_RANGED") {
    return ItemType::WEAPON_RANGED;
  } else if (itemTypeString == "WEAPON_AMMO") {
    return ItemType::WEAPON_AMMO;
  } else if (itemTypeString == "SHIELD") {
    return ItemType::SHIELD;
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
  } else if (itemTypeString == "RUNE") {
    return ItemType::RUNE;
  }
  return ItemType::UNKNOWN;
}

bool itemTypeIsEquippable(ItemType itemType) {
  // RUNE uses rune slots only (itemTypeUsesRuneSlots), not armor/weapon maps.
  switch (itemType) {
  case ItemType::WEAPON_MELEE:
  case ItemType::WEAPON_MELEE_2H:
  case ItemType::WEAPON_RANGED:
  case ItemType::WEAPON_AMMO:
  case ItemType::SHIELD:
  case ItemType::GARB:
  case ItemType::PANTS:
  case ItemType::GLOVES:
  case ItemType::HAT:
  case ItemType::SHOES:
  case ItemType::NECKLACE:
    return true;
  default:
    return false;
  }
}

bool itemTypeIsTwoHandedWeapon(ItemType itemType) {
  return itemType == ItemType::WEAPON_MELEE_2H;
}

bool itemTypeUsesWeaponSlots(ItemType itemType) {
  switch (itemType) {
  case ItemType::WEAPON_MELEE:
  case ItemType::WEAPON_MELEE_2H:
  case ItemType::WEAPON_RANGED:
    return true;
  default:
    return false;
  }
}

bool itemTypeUsesRuneSlots(ItemType itemType) {
  return itemType == ItemType::RUNE;
}

ItemUsability getItemUsabilityFromString(const bmin::String& value) {
  if (value == "USABLE_EVERYWHERE") {
    return ItemUsability::USABLE_EVERYWHERE;
  }
  if (value == "USABLE_TOWN_ONLY") {
    return ItemUsability::USABLE_TOWN_ONLY;
  }
  if (value == "USABLE_COMBAT_ONLY") {
    return ItemUsability::USABLE_COMBAT_ONLY;
  }
  if (value == "USABLE_OUTSIDE_ONLY") {
    return ItemUsability::USABLE_OUTSIDE_ONLY;
  }
  if (value == "USABLE_TOWN_AND_COMBAT") {
    return ItemUsability::USABLE_TOWN_AND_COMBAT;
  }
  return ItemUsability::NOT_USABLE;
}

} // namespace model
