#pragma once

#include "model/templates/CombatTypes.h"
#include <optional>
#include <string>
#include <vector>

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

std::string getStringFromItemType(ItemType itemType);
ItemType getItemTypeFromString(const std::string& itemTypeString);

ItemUsability getItemUsabilityFromString(const std::string& value);

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

struct ItemWeaponConfig {
  std::string abilityName;
  std::vector<AbilityAttackDmg> dmgOverrides;
};

struct ItemUseAbilityConfig {
  std::string abilityName;
  std::vector<AbilityAttackDmg> dmgOverrides;
  std::vector<AbilityRestore> restoreOverrides;
};

struct ItemTemplate {
  ItemType itemType = ItemType::UNKNOWN;
  std::string name;
  std::string label;
  std::string iconSpriteName;
  std::string description;
  int weight = 0;
  int value = 0;
  bool stackable = false;
  ItemUsability itemUsability = ItemUsability::NOT_USABLE;
  std::optional<ItemUseAbilityConfig> useAbility;
  std::vector<std::string> statusEffectNames;
  std::optional<ItemWeaponConfig> weapon;
};

} // namespace model
