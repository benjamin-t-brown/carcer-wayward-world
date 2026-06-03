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

enum class ItemUsabilityType { ITEM_USE_DEFAULT, ITEM_USE_CAST_SPELL };

struct ItemUsabilityArgs {
  ItemUsabilityType itemUsabilityType = ItemUsabilityType::ITEM_USE_DEFAULT;
  std::vector<int> intArgs;
  std::vector<std::string> stringArgs;
};

struct ItemWeaponConfig {
  std::string abilityName;
  std::vector<AbilityAttackDmg> dmgOverrides;
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
  ItemUsabilityArgs itemUsabilityArgs;
  std::vector<std::string> statusEffectNames;
  std::optional<ItemWeaponConfig> weapon;
};

} // namespace model
