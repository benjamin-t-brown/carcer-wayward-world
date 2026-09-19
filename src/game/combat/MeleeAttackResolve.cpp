#include "game/combat/MeleeAttackResolve.h"

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "game/combat/AbilityOverrideMerge.h"
#include "game/diceHelpers.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/Player.h"
#include "model/stats/CharacterDerivedStats.h"
#include "model/stats/CharacterStats.h"
#include "sdl2w/Logger.h"

#include <optional>

namespace game {

namespace {

bool isMeleeWeaponType(model::ItemType itemType) {
  return itemType == model::ItemType::WEAPON_MELEE ||
         itemType == model::ItemType::WEAPON_MELEE_2H;
}

const model::CharacterInventoryItem*
findInventoryItemById(const model::CharacterPlayer& character,
                      const bmin::String& itemId) {
  for (const auto& item : character.inventory) {
    if (item.id == itemId) {
      return &item;
    }
  }
  return nullptr;
}

std::optional<model::AbilityTemplate>
copyDefaultMeleeAbility(const db::Database& database) {
  const auto* found = database.findAbilityTemplate(kMeleeAttackDefaultName);
  if (found == nullptr) {
    LOG(WARN) << "MeleeAttackResolve: missing ability " << kMeleeAttackDefaultName
              << LOG_ENDL;
    return std::nullopt;
  }
  return *found;
}

std::optional<ResolvedMeleeAbility>
resolveWeaponSlot(const model::CharacterPlayer& partyMember,
                  const bmin::String& weaponId,
                  const db::Database& database) {
  if (weaponId.empty()) {
    return std::nullopt;
  }
  const auto* inventoryItem = findInventoryItemById(partyMember, weaponId);
  if (inventoryItem == nullptr) {
    return std::nullopt;
  }
  const auto* itemTemplate =
      database.findItemTemplate(bmin::toStringView(inventoryItem->itemName));
  if (itemTemplate == nullptr) {
    return std::nullopt;
  }
  if (!isMeleeWeaponType(itemTemplate->itemType)) {
    return std::nullopt;
  }
  if (!itemTemplate->weapon.has_value()) {
    return std::nullopt;
  }
  const auto& weapon = *itemTemplate->weapon;
  if (weapon.abilityName.empty()) {
    auto ability = copyDefaultMeleeAbility(database);
    if (!ability.has_value()) {
      return std::nullopt;
    }
    return ResolvedMeleeAbility{std::move(*ability), false};
  }
  const auto* namedAbility =
      database.findAbilityTemplate(bmin::toStringView(weapon.abilityName));
  if (namedAbility == nullptr) {
    auto ability = copyDefaultMeleeAbility(database);
    if (!ability.has_value()) {
      return std::nullopt;
    }
    return ResolvedMeleeAbility{std::move(*ability), false};
  }
  auto resolved = model::AbilityTemplate{*namedAbility};
  mergeAbilityAttackDmgOverrides(resolved, weapon.dmgOverrides);
  return ResolvedMeleeAbility{std::move(resolved), false};
}

} // namespace

bmin::DynArray<ResolvedMeleeAbility> resolveMeleeAttackAbilities(
    model::Player& player,
    const model::CharacterInstance& attacker,
    const db::Database& database) {
  auto resolved = bmin::DynArray<ResolvedMeleeAbility>{};
  auto* partyMember = model::playerFindPartyMemberById(player, attacker.id);
  if (partyMember != nullptr) {
    auto hand0 =
        resolveWeaponSlot(*partyMember, partyMember->equipment.weapon0Id, database);
    auto hand1 =
        resolveWeaponSlot(*partyMember, partyMember->equipment.weapon1Id, database);
    if (hand0.has_value()) {
      hand0->offHand = false;
      resolved.pushBack(std::move(*hand0));
    }
    if (hand1.has_value()) {
      hand1->offHand = !resolved.empty();
      resolved.pushBack(std::move(*hand1));
    }
  }
  if (resolved.empty()) {
    auto ability = copyDefaultMeleeAbility(database);
    if (ability.has_value()) {
      resolved.pushBack(ResolvedMeleeAbility{std::move(*ability), false, true});
    }
  }
  return resolved;
}

int weaponMasteryBonus(const model::CharacterStats& stats,
                       const model::AbilityAttack& attack,
                       bool unarmed) {
  const auto& weapon = stats.trainable.weapon;
  if (unarmed) {
    return weapon.unarmed;
  }
  if (attack.attackClass == model::AttackClass::ATTACK_CLASS_RANGED) {
    return weapon.range;
  }
  switch (attack.damageType) {
  case model::DamageType::DAMAGE_TYPE_EDGED:
    return weapon.edged;
  case model::DamageType::DAMAGE_TYPE_BASHING:
    return weapon.blunt;
  case model::DamageType::DAMAGE_TYPE_PIERCING:
    return weapon.pole;
  default:
    return 0;
  }
}

int meleeTargetArmorClass(const model::CharacterStats& defenderStats) {
  return model::computeCharacterDerivedStats(defenderStats).armorClass;
}

bool meleeAttackHits(int d20Roll, int attackBonus, int targetArmorClass) {
  if (d20Roll <= 1) {
    return false;
  }
  if (d20Roll >= 20) {
    return true;
  }
  return (d20Roll + attackBonus) >= targetArmorClass;
}

bool rollMeleeAttackHit(model::AttackClass attackClass,
                        int attackBonus,
                        int targetArmorClass,
                        bool applyOffHandPenalty) {
  if (attackClass == model::AttackClass::ATTACK_CLASS_AUTO_HIT) {
    LOG(DEBUG) << "MeleeAttack: to-hit AUTO_HIT class="
               << model::attackClassToString(attackClass) << LOG_ENDL;
    return true;
  }
  const auto rollA = rollDice(model::Dice::D20);
  auto usedRoll = rollA;
  if (applyOffHandPenalty) {
    const auto rollB = rollDice(model::Dice::D20);
    usedRoll = rollA < rollB ? rollA : rollB;
    LOG(DEBUG) << "MeleeAttack: to-hit d20=" << rollA << "," << rollB
               << " disadvantage=" << usedRoll << " bonus=" << attackBonus
               << " total=" << (usedRoll + attackBonus) << " vs AC " << targetArmorClass
               << " class=" << model::attackClassToString(attackClass) << " result="
               << (meleeAttackHits(usedRoll, attackBonus, targetArmorClass) ? "hit"
                                                                            : "miss")
               << LOG_ENDL;
  } else {
    LOG(DEBUG) << "MeleeAttack: to-hit d20=" << usedRoll << " bonus=" << attackBonus
               << " total=" << (usedRoll + attackBonus) << " vs AC " << targetArmorClass
               << " class=" << model::attackClassToString(attackClass) << " result="
               << (meleeAttackHits(usedRoll, attackBonus, targetArmorClass) ? "hit"
                                                                            : "miss")
               << LOG_ENDL;
  }
  return meleeAttackHits(usedRoll, attackBonus, targetArmorClass);
}

} // namespace game
