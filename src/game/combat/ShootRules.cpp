#include "game/combat/ShootRules.h"

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "game/combat/AbilityOverrideMerge.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"

namespace game {

namespace {

const model::CharacterPlayer* findPartyMember(const model::Player& player,
                                              const bmin::String& id) {
  for (size_t i = 0; i < player.party.size(); ++i) {
    if (player.party[i].instanceId == id) {
      return &player.party[i];
    }
  }
  return nullptr;
}

model::CharacterPlayer* findPartyMember(model::Player& player, const bmin::String& id) {
  for (size_t i = 0; i < player.party.size(); ++i) {
    if (player.party[i].instanceId == id) {
      return &player.party[i];
    }
  }
  return nullptr;
}

const model::CharacterInventoryItem*
findInventoryItemById(const model::CharacterPlayer& character, const bmin::String& itemId) {
  if (itemId.empty()) {
    return nullptr;
  }
  for (size_t i = 0; i < character.inventory.size(); ++i) {
    if (character.inventory[i].id == itemId) {
      return &character.inventory[i];
    }
  }
  return nullptr;
}

int shooterMana(const model::Player& player, const model::CharacterInstance& shooter) {
  if (const auto* member = findPartyMember(player, shooter.id)) {
    return member->currentMp;
  }
  return shooter.currentMp;
}

std::optional<model::AbilityTemplate>
resolveRangedSlot(const model::CharacterPlayer& member,
                  const bmin::String& weaponId,
                  const db::Database& database) {
  if (weaponId.empty()) {
    return std::nullopt;
  }
  const auto* inventoryItem = findInventoryItemById(member, weaponId);
  if (inventoryItem == nullptr) {
    return std::nullopt;
  }
  const auto* itemTemplate =
      database.findItemTemplate(bmin::toStringView(inventoryItem->itemName));
  if (itemTemplate == nullptr || itemTemplate->itemType != model::ItemType::WEAPON_RANGED) {
    return std::nullopt;
  }
  if (!itemTemplate->weapon.has_value() || itemTemplate->weapon->abilityName.empty()) {
    return std::nullopt;
  }
  const auto* namedAbility = database.findAbilityTemplate(
      bmin::toStringView(itemTemplate->weapon->abilityName));
  if (namedAbility == nullptr) {
    return std::nullopt;
  }
  auto resolved = model::AbilityTemplate{*namedAbility};
  mergeAbilityAttackDmgOverrides(resolved, itemTemplate->weapon->dmgOverrides);
  return resolved;
}

bool ammoStackCoversCost(const model::CharacterPlayer& member, int costValue) {
  if (costValue <= 0) {
    return true;
  }
  const auto* ammo = findInventoryItemById(member, member.equipment.ammoId);
  return ammo != nullptr && ammo->quantity >= costValue;
}

} // namespace

std::optional<ResolvedShootAbility> resolveEquippedShootAbility(
    const model::Player& player,
    const model::CharacterInstance& shooter,
    const db::Database& database) {
  const auto* member = findPartyMember(player, shooter.id);
  if (member == nullptr) {
    return std::nullopt;
  }
  if (auto mainHand = resolveRangedSlot(*member, member->equipment.weapon0Id, database)) {
    return ResolvedShootAbility{std::move(*mainHand)};
  }
  if (auto offHand = resolveRangedSlot(*member, member->equipment.weapon1Id, database)) {
    return ResolvedShootAbility{std::move(*offHand)};
  }
  return std::nullopt;
}

bool canPayShootAuxCost(const model::Player& player,
                        const model::CharacterInstance& shooter,
                        const model::AbilityTemplate& ability) {
  const auto cost = ability.costValue;
  switch (ability.costType) {
  case model::AbilityCostType::ABILITY_COST_NONE:
    return true;
  case model::AbilityCostType::ABILITY_COST_COOLDOWN:
    return false;
  case model::AbilityCostType::ABILITY_COST_AMMO: {
    if (cost <= 0) {
      return true;
    }
    const auto* member = findPartyMember(player, shooter.id);
    return member != nullptr && ammoStackCoversCost(*member, cost);
  }
  case model::AbilityCostType::ABILITY_COST_HP:
    return cost <= 0 || model::getCharacterHp(player, shooter) >= cost;
  case model::AbilityCostType::ABILITY_COST_MANA:
    return cost <= 0 || shooterMana(player, shooter) >= cost;
  }
  return false;
}

bool canStartShoot(const model::Player& player,
                   const model::CharacterInstance& shooter,
                   const db::Database& database) {
  const auto resolved = resolveEquippedShootAbility(player, shooter, database);
  if (!resolved.has_value()) {
    return false;
  }
  if (shooter.currentAp <= 0) {
    return false;
  }
  return canPayShootAuxCost(player, shooter, resolved->ability);
}

ShootAuxSpendResult spendShootAuxCost(model::Player& player,
                                      model::CharacterInstance& shooter,
                                      const model::AbilityTemplate& ability) {
  auto result = ShootAuxSpendResult{};
  if (!canPayShootAuxCost(player, shooter, ability)) {
    return result;
  }
  result.ok = true;
  const auto cost = ability.costValue;
  if (cost <= 0) {
    return result;
  }
  switch (ability.costType) {
  case model::AbilityCostType::ABILITY_COST_NONE:
  case model::AbilityCostType::ABILITY_COST_COOLDOWN:
    return result;
  case model::AbilityCostType::ABILITY_COST_HP:
    result.hpCost = cost;
    return result;
  case model::AbilityCostType::ABILITY_COST_MANA: {
    if (auto* member = findPartyMember(player, shooter.id)) {
      member->currentMp -= cost;
    } else {
      shooter.currentMp -= cost;
    }
    return result;
  }
  case model::AbilityCostType::ABILITY_COST_AMMO: {
    auto* member = findPartyMember(player, shooter.id);
    if (member == nullptr) {
      result.ok = false;
      return result;
    }
    const auto ammoId = member->equipment.ammoId;
    model::characterPlayerRemoveItemFromInventoryById(*member, ammoId, cost);
    if (findInventoryItemById(*member, ammoId) == nullptr) {
      member->equipment.ammoId.clear();
    }
    return result;
  }
  }
  result.ok = false;
  return result;
}

} // namespace game
