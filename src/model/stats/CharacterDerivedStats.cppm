module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>

export module carcer.model.templates:CharacterDerivedStats;
export import bmin.containers;
import bmin.string_interop;
export import :CharacterStats;
import sdl2w;
#include "macros.h"

export {

// --- from model/stats/CharacterDerivedStats.h ---
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

} // export

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
