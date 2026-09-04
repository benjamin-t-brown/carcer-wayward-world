module;
#include <cstddef>
#include <cstdint>
#include <utility>

module carcer.game.combat;
import carcer.game.diceHelpers;
import carcer.model.instances.CharacterInstance;
import carcer.model.templates;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace game {

CalculatedAbilityDamageResult calculateAttackDamage(
    const model::AbilityAttack& attack,
    const model::CharacterInstance& attacker,
    const model::CharacterInstance& target) {
  return CalculatedAbilityDamageResult{true, 10, 0};
}

CalculatedAbilityDamageResult calculateAbilityDamage(
    const model::AbilityDamage& abilityDamage,
    const model::CharacterInstance& attacker,
    const model::CharacterInstance& victim) {
  int damage = abilityDamage.dmgBonus;
  damage += rollDiceList(abilityDamage.dmgDice);

  auto& attackerStats = attacker.stats;
  // TODO DR for damage type
  // auto& victimStats = victim.stats;

  auto mult = abilityDamage.dmgStatMult;

  switch (abilityDamage.dmgStat) {
  case model::StatsEnum::STAT_STR:
    damage += attackerStats.generic.str * mult;
    break;
  case model::StatsEnum::STAT_MND:
    damage += attackerStats.generic.mnd * mult;
    break;
  case model::StatsEnum::STAT_CON:
    damage += attackerStats.generic.con * mult;
    break;
  case model::StatsEnum::STAT_AGI:
    damage += attackerStats.generic.agi * mult;
    break;
  case model::StatsEnum::STAT_LCK:
    damage += attackerStats.generic.lck * mult;
    break;
  default:
    break;
  }

  return CalculatedAbilityDamageResult{true, damage, 0};
}

} // namespace game
