#pragma once

#include "model/stats/CharacterDerivedStats.h"
#include "bmin/String.h"
namespace model {

struct CharacterDerivedStatDefinitions {
  static bmin::String derivedTitle();
  static bmin::String derivedSkillsTitle();

  static bmin::String actionPointsLabel();
  static bmin::String actionPointsDescription();
  static bmin::String actionPointsValue(const CharacterDerivedStats& derived);

  static bmin::String mightDamageLabel();
  static bmin::String mightDamageDescription();
  static bmin::String mightDamageValue(const CharacterDerivedStats& derived);

  static bmin::String magicDamageLabel();
  static bmin::String magicDamageDescription();
  static bmin::String magicDamageValue(const CharacterDerivedStats& derived);

  static bmin::String hpLabel();
  static bmin::String hpDescription();
  static bmin::String hpValue(const CharacterDerivedStats& derived);

  static bmin::String attackHitChanceLabel();
  static bmin::String attackHitChanceDescription();
  static bmin::String attackHitChanceValue(const CharacterDerivedStats& derived);

  static bmin::String abilityPowerLabel();
  static bmin::String abilityPowerDescription();
  static bmin::String abilityPowerValue(const CharacterDerivedStats& derived);

  static bmin::String damageReductionLabel();
  static bmin::String damageReductionDescription();
  static bmin::String damageReductionValue(const CharacterDerivedStats& derived);

  static bmin::String armorClassLabel();
  static bmin::String armorClassDescription();
  static bmin::String armorClassValue(const CharacterDerivedStats& derived);

  static bmin::String spellPotencyLabel();
  static bmin::String spellPotencyDescription();
  static bmin::String spellPotencyValue(const CharacterDerivedStats& derived);

  static bmin::String manaLabel();
  static bmin::String manaDescription();
  static bmin::String manaValue(const CharacterDerivedStats& derived);

  static bmin::String resistancesLabel();
  static bmin::String resistancesDescription();
  static bmin::String resistancesValue(const CharacterDerivedStats& derived);

  static bmin::String jumpDistanceLabel();
  static bmin::String jumpDistanceDescription();
  static bmin::String jumpDistanceValue(const CharacterDerivedStats& derived);

  static bmin::String healingEffectivenessLabel();
  static bmin::String healingEffectivenessDescription();
  static bmin::String healingEffectivenessValue(const CharacterDerivedStats& derived);

  static bmin::String statusEffectShieldLabel();
  static bmin::String statusEffectShieldDescription();
  static bmin::String statusEffectShieldValue(const CharacterDerivedStats& derived);

  static bmin::String materiaSlotsLabel();
  static bmin::String materiaSlotsDescription();
  static bmin::String materiaSlotsValue(const CharacterDerivedStats& derived);

  static bmin::String shieldBonusLabel();
  static bmin::String shieldBonusDescription();
  static bmin::String shieldBonusValue(const CharacterDerivedStats& derived);

  static bmin::String enemyVisionRangeLabel();
  static bmin::String enemyVisionRangeDescription();
  static bmin::String enemyVisionRangeValue(const CharacterDerivedStats& derived);

  static bmin::String mageLoreLabel();
  static bmin::String mageLoreDescription();
  static bmin::String mageLoreValue(const CharacterDerivedStats& derived);

  static bmin::String toolUseLabel();
  static bmin::String toolUseDescription();
  static bmin::String toolUseValue(const CharacterDerivedStats& derived);

  static bmin::String tradeDiscountLabel();
  static bmin::String tradeDiscountDescription();
  static bmin::String tradeDiscountValue(const CharacterDerivedStats& derived);

  static bmin::String itemUsageLabel();
  static bmin::String itemUsageDescription();
  static bmin::String itemUsageValue(const CharacterDerivedStats& derived);

  static bmin::String foodConsumptionLabel();
  static bmin::String foodConsumptionDescription();
  static bmin::String foodConsumptionValue(const CharacterDerivedStats& derived);

  static bmin::String firstAidLabel();
  static bmin::String firstAidDescription();
  static bmin::String firstAidValue(const CharacterDerivedStats& derived);

  static bmin::String ingredientFindChanceLabel();
  static bmin::String ingredientFindChanceDescription();
  static bmin::String ingredientFindChanceValue(const CharacterDerivedStats& derived);

  static bmin::String foodConsumptionPerDayLabel();
  static bmin::String foodConsumptionPerDayDescription();
  static bmin::String foodConsumptionPerDayValue(const CharacterDerivedStats& derived);
};

} // namespace model
