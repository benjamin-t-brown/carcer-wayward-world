#include "model/stats/CharacterDerivedStats.h"
#include <algorithm>

namespace model {

CharacterDerivedStats computeCharacterDerivedStats(const CharacterStats& stats,
                                                   int characterLevel) {
  CharacterDerivedStats derived;
  const auto& generic = stats.generic;
  const auto& trainable = stats.trainable;
  const auto& skills = stats.skills;

  derived.actionPoints = 1;
  derived.mightDamage = 1 + generic.str;
  derived.magicDamage = generic.mnd;
  derived.hp = 10 + generic.con * 5;
  derived.attackHitChancePercent = 50;
  derived.abilityPower = trainable.magic.abilityPower;
  derived.damageReduction = trainable.body.dr;
  derived.armorClass = 10;
  derived.spellPotency = 1;
  derived.maxMana = 10 + trainable.magic.mana + skills.focus * 4;
  derived.jumpDistance = 2;
  derived.healingEffectivenessPercent = 100;
  derived.statusEffectShield = 0;
  derived.materiaSlots = 0;
  derived.shieldBonus = 0;

  derived.enemyVisionRange = skills.stealth;
  derived.mageLore = skills.magicItemUse;
  derived.toolUse = skills.trickery;
  derived.tradeDiscountPercent = skills.social;
  derived.itemUsagePercent = 100;
  derived.foodConsumption = std::max(0, 100 - skills.cooking);
  derived.firstAidPerLevel = std::max(1, characterLevel);
  derived.ingredientFindChancePercent = 10 + skills.survival;
  derived.foodConsumptionPerDay = 100;

  return derived;
}

} // namespace model
