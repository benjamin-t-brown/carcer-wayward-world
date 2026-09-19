#include "game/combat/Damage.h"
#include "game/diceHelpers.h"
#include "sdl2w/Logger.h"

#include <cmath>

namespace game {

namespace {

int attackDamageStatValue(const model::CharacterStats& stats, model::StatsEnum which) {
  switch (which) {
  case model::StatsEnum::STAT_STR:
    return stats.generic.str;
  case model::StatsEnum::STAT_MND:
    return stats.generic.mnd;
  case model::StatsEnum::STAT_CON:
    return stats.generic.con;
  case model::StatsEnum::STAT_AGI:
    return stats.generic.agi;
  case model::StatsEnum::STAT_LCK:
    return stats.generic.lck;
  }
  return 0;
}

} // namespace

CalculatedAbilityDamageResult calculateAttackDamage(
    const model::AbilityAttack& attack,
    const model::CharacterStats& attackerStats) {
  if (!attack.dmg.has_value()) {
    LOG(DEBUG) << "AttackDamage: no dmg block, damage=0" << LOG_ENDL;
    return CalculatedAbilityDamageResult{true, 0, 0};
  }
  const auto& dmg = *attack.dmg;
  const auto diceRoll = rollDiceList(dmg.dmgDice);
  const auto statValue = attackDamageStatValue(attackerStats, dmg.dmgStat);
  const float total = static_cast<float>(diceRoll + dmg.dmgBonus) +
                      static_cast<float>(statValue) * dmg.dmgStatMult;
  const auto damage = int{static_cast<int>(std::lround(total))};
  LOG(DEBUG) << "AttackDamage: dice=" << diceRoll << " bonus=" << dmg.dmgBonus << " "
             << model::statsEnumToString(dmg.dmgStat) << "=" << statValue << " * "
             << dmg.dmgStatMult << " total=" << damage << LOG_ENDL;
  return CalculatedAbilityDamageResult{true, damage, 0};
}

CalculatedAbilityDamageResult calculateAbilityDamage(
    const model::AbilityDamage& abilityDamage,
    const model::CharacterStats& attackerStats) {
  const auto diceRoll = rollDiceList(abilityDamage.dmgDice);
  const auto statValue = attackDamageStatValue(attackerStats, abilityDamage.dmgStat);
  const float total = static_cast<float>(diceRoll + abilityDamage.dmgBonus) +
                      static_cast<float>(statValue) * abilityDamage.dmgStatMult;
  const auto damage = int{static_cast<int>(std::lround(total))};
  LOG(DEBUG) << "AbilityDamage: dice=" << diceRoll << " bonus=" << abilityDamage.dmgBonus
             << " " << model::statsEnumToString(abilityDamage.dmgStat) << "=" << statValue
             << " * " << abilityDamage.dmgStatMult << " total=" << damage << LOG_ENDL;
  return CalculatedAbilityDamageResult{true, damage, 0};
}

CalculatedAbilityDamageResult calculateAbilityDamage(
    const model::AbilityDamage& abilityDamage,
    const model::CharacterInstance& attacker,
    const model::CharacterInstance& /*victim*/) {
  return calculateAbilityDamage(abilityDamage, attacker.stats);
}

int calculateAbilityTemplateDamage(const model::AbilityTemplate& ability,
                                   const model::CharacterStats& attackerStats) {
  auto damage = int{0};
  for (const auto& abilityDamage : ability.damages) {
    damage += calculateAbilityDamage(abilityDamage, attackerStats).damage;
  }
  for (const auto& attack : ability.attacks) {
    damage += calculateAttackDamage(attack, attackerStats).damage;
  }
  return damage;
}

int calculateAbilityRestoreAmount(const model::AbilityRestore& restore,
                                  const model::CharacterStats& casterStats) {
  const auto diceRoll = rollDiceList(restore.restoreDice);
  const auto statValue = attackDamageStatValue(casterStats, restore.restoreStat);
  const auto total = diceRoll + restore.restoreBonus + statValue * restore.restoreStatMult;
  LOG(DEBUG) << "AbilityRestore: dice=" << diceRoll << " bonus=" << restore.restoreBonus
             << " " << model::statsEnumToString(restore.restoreStat) << "=" << statValue
             << " * " << restore.restoreStatMult << " total=" << total << LOG_ENDL;
  return total;
}

int calculateAbilityTemplateHpDelta(const model::AbilityTemplate& ability,
                                    const model::CharacterStats& casterStats) {
  auto hpDelta = int{0};
  for (const auto& restore : ability.restores) {
    if (restore.restoreWhich == model::CurrentStatEnum::CURRENT_STAT_HP) {
      hpDelta += calculateAbilityRestoreAmount(restore, casterStats);
    }
  }
  hpDelta -= calculateAbilityTemplateDamage(ability, casterStats);
  return hpDelta;
}

} // namespace game
