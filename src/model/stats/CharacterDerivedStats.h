#pragma once

#include "model/stats/CharacterStats.h"
#include "bmin/String.h"
namespace model {

struct CharacterDerivedStats {
  int hp = 10;
  int maxMana = 10;
  int actionPoints = 1;
  int mightDamage = 1;
  int magicDamage = 0;
  int attackHitChancePercent = 50;
  int abilityPower = 0;
  int damageReduction = 0;
  int armorClass = 10;
  int spellPotency = 1;
  bmin::String resistancesSummary = "None";
  int jumpDistance = 2;
  int healingEffectivenessPercent = 100;
  int statusEffectShield = 0;
  int materiaSlots = 0;
  int shieldBonus = 0;
  int enemyVisionRange = 0;
  int mageLore = 0;
  int toolUse = 0;
  int tradeDiscountPercent = 0;
  int itemUsagePercent = 100;
  int foodConsumption = 100;
  int firstAidPerLevel = 1;
  int ingredientFindChancePercent = 10;
  int foodConsumptionPerDay = 100;
};

CharacterDerivedStats computeCharacterDerivedStats(const CharacterStats& stats,
                                                   int characterLevel = 1);

} // namespace model
