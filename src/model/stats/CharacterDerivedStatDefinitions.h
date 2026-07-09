#pragma once

#include "model/stats/CharacterDerivedStats.h"
#include "lib/Types.h"

namespace model {

struct CharacterDerivedStatDefinitions {
  static String derivedTitle();
  static String derivedSkillsTitle();

  static String actionPointsLabel();
  static String actionPointsDescription();
  static String actionPointsValue(const CharacterDerivedStats& derived);

  static String mightDamageLabel();
  static String mightDamageDescription();
  static String mightDamageValue(const CharacterDerivedStats& derived);

  static String magicDamageLabel();
  static String magicDamageDescription();
  static String magicDamageValue(const CharacterDerivedStats& derived);

  static String hpLabel();
  static String hpDescription();
  static String hpValue(const CharacterDerivedStats& derived);

  static String attackHitChanceLabel();
  static String attackHitChanceDescription();
  static String attackHitChanceValue(const CharacterDerivedStats& derived);

  static String abilityPowerLabel();
  static String abilityPowerDescription();
  static String abilityPowerValue(const CharacterDerivedStats& derived);

  static String damageReductionLabel();
  static String damageReductionDescription();
  static String damageReductionValue(const CharacterDerivedStats& derived);

  static String armorClassLabel();
  static String armorClassDescription();
  static String armorClassValue(const CharacterDerivedStats& derived);

  static String spellPotencyLabel();
  static String spellPotencyDescription();
  static String spellPotencyValue(const CharacterDerivedStats& derived);

  static String manaLabel();
  static String manaDescription();
  static String manaValue(const CharacterDerivedStats& derived);

  static String resistancesLabel();
  static String resistancesDescription();
  static String resistancesValue(const CharacterDerivedStats& derived);

  static String jumpDistanceLabel();
  static String jumpDistanceDescription();
  static String jumpDistanceValue(const CharacterDerivedStats& derived);

  static String healingEffectivenessLabel();
  static String healingEffectivenessDescription();
  static String healingEffectivenessValue(const CharacterDerivedStats& derived);

  static String statusEffectShieldLabel();
  static String statusEffectShieldDescription();
  static String statusEffectShieldValue(const CharacterDerivedStats& derived);

  static String materiaSlotsLabel();
  static String materiaSlotsDescription();
  static String materiaSlotsValue(const CharacterDerivedStats& derived);

  static String shieldBonusLabel();
  static String shieldBonusDescription();
  static String shieldBonusValue(const CharacterDerivedStats& derived);

  static String enemyVisionRangeLabel();
  static String enemyVisionRangeDescription();
  static String enemyVisionRangeValue(const CharacterDerivedStats& derived);

  static String mageLoreLabel();
  static String mageLoreDescription();
  static String mageLoreValue(const CharacterDerivedStats& derived);

  static String toolUseLabel();
  static String toolUseDescription();
  static String toolUseValue(const CharacterDerivedStats& derived);

  static String tradeDiscountLabel();
  static String tradeDiscountDescription();
  static String tradeDiscountValue(const CharacterDerivedStats& derived);

  static String itemUsageLabel();
  static String itemUsageDescription();
  static String itemUsageValue(const CharacterDerivedStats& derived);

  static String foodConsumptionLabel();
  static String foodConsumptionDescription();
  static String foodConsumptionValue(const CharacterDerivedStats& derived);

  static String firstAidLabel();
  static String firstAidDescription();
  static String firstAidValue(const CharacterDerivedStats& derived);

  static String ingredientFindChanceLabel();
  static String ingredientFindChanceDescription();
  static String ingredientFindChanceValue(const CharacterDerivedStats& derived);

  static String foodConsumptionPerDayLabel();
  static String foodConsumptionPerDayDescription();
  static String foodConsumptionPerDayValue(const CharacterDerivedStats& derived);
};

} // namespace model
