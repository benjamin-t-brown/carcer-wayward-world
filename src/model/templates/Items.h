#pragma once

#include "model/templates/AbilityTypes.h"
#include <optional>
#include "lib/Types.h"

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

String getStringFromItemType(ItemType itemType);
ItemType getItemTypeFromString(const String& itemTypeString);

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

ItemUsability getItemUsabilityFromString(const String& value);

struct ItemWeaponConfig {
  String abilityName;
  bmin::DynArray<AbilityAttackDmg> dmgOverrides;
};

struct ItemUseAbilityConfig {
  String abilityName;
  bmin::DynArray<AbilityAttackDmg> dmgOverrides;
  bmin::DynArray<AbilityRestore> restoreOverrides;
};

struct ItemTemplate {
  ItemType itemType = ItemType::UNKNOWN;
  String name;
  String label;
  String iconSpriteName;
  String description;
  int weight = 0;
  int value = 0;
  bool stackable = false;
  bool indestructable = false;
  ItemUsability itemUsability = ItemUsability::NOT_USABLE;
  std::optional<ItemUseAbilityConfig> useAbility;
  std::optional<String> useSpecialEvent;
  bmin::DynArray<String> statusEffectNames;
  std::optional<ItemWeaponConfig> weapon;
};

} // namespace model
