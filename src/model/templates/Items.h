#pragma once

#include "model/templates/AbilityTypes.h"
#include <optional>
#include "bmin/DynArray.h"
#include "bmin/String.h"

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
  UNKNOWN
};

bmin::String getStringFromItemType(ItemType itemType);
ItemType getItemTypeFromString(const bmin::String& itemTypeString);

bool itemTypeIsEquippable(ItemType itemType);
bool itemTypeIsTwoHandedWeapon(ItemType itemType);
bool itemTypeUsesWeaponSlots(ItemType itemType);

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
};

} // namespace model
