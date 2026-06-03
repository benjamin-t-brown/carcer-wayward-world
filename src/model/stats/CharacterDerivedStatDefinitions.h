#pragma once

#include "model/stats/CharacterDerivedStats.h"
#include <string>

namespace model {

struct CharacterDerivedStatDefinitions {
  static std::string derivedTitle();
  static std::string derivedSkillsTitle();

  static std::string actionPointsLabel();
  static std::string actionPointsDescription();
  static std::string actionPointsValue(const CharacterDerivedStats& derived);

  static std::string mightDamageLabel();
  static std::string mightDamageDescription();
  static std::string mightDamageValue(const CharacterDerivedStats& derived);

  static std::string magicDamageLabel();
  static std::string magicDamageDescription();
  static std::string magicDamageValue(const CharacterDerivedStats& derived);

  static std::string hpLabel();
  static std::string hpDescription();
  static std::string hpValue(const CharacterDerivedStats& derived);

  static std::string attackHitChanceLabel();
  static std::string attackHitChanceDescription();
  static std::string attackHitChanceValue(const CharacterDerivedStats& derived);

  static std::string abilityPowerLabel();
  static std::string abilityPowerDescription();
  static std::string abilityPowerValue(const CharacterDerivedStats& derived);

  static std::string damageReductionLabel();
  static std::string damageReductionDescription();
  static std::string damageReductionValue(const CharacterDerivedStats& derived);

  static std::string armorClassLabel();
  static std::string armorClassDescription();
  static std::string armorClassValue(const CharacterDerivedStats& derived);

  static std::string spellPotencyLabel();
  static std::string spellPotencyDescription();
  static std::string spellPotencyValue(const CharacterDerivedStats& derived);

  static std::string manaLabel();
  static std::string manaDescription();
  static std::string manaValue(const CharacterDerivedStats& derived);

  static std::string resistancesLabel();
  static std::string resistancesDescription();
  static std::string resistancesValue(const CharacterDerivedStats& derived);

  static std::string jumpDistanceLabel();
  static std::string jumpDistanceDescription();
  static std::string jumpDistanceValue(const CharacterDerivedStats& derived);

  static std::string healingEffectivenessLabel();
  static std::string healingEffectivenessDescription();
  static std::string healingEffectivenessValue(const CharacterDerivedStats& derived);

  static std::string statusEffectShieldLabel();
  static std::string statusEffectShieldDescription();
  static std::string statusEffectShieldValue(const CharacterDerivedStats& derived);

  static std::string materiaSlotsLabel();
  static std::string materiaSlotsDescription();
  static std::string materiaSlotsValue(const CharacterDerivedStats& derived);

  static std::string shieldBonusLabel();
  static std::string shieldBonusDescription();
  static std::string shieldBonusValue(const CharacterDerivedStats& derived);

  static std::string enemyVisionRangeLabel();
  static std::string enemyVisionRangeDescription();
  static std::string enemyVisionRangeValue(const CharacterDerivedStats& derived);

  static std::string mageLoreLabel();
  static std::string mageLoreDescription();
  static std::string mageLoreValue(const CharacterDerivedStats& derived);

  static std::string toolUseLabel();
  static std::string toolUseDescription();
  static std::string toolUseValue(const CharacterDerivedStats& derived);

  static std::string tradeDiscountLabel();
  static std::string tradeDiscountDescription();
  static std::string tradeDiscountValue(const CharacterDerivedStats& derived);

  static std::string itemUsageLabel();
  static std::string itemUsageDescription();
  static std::string itemUsageValue(const CharacterDerivedStats& derived);

  static std::string foodConsumptionLabel();
  static std::string foodConsumptionDescription();
  static std::string foodConsumptionValue(const CharacterDerivedStats& derived);

  static std::string firstAidLabel();
  static std::string firstAidDescription();
  static std::string firstAidValue(const CharacterDerivedStats& derived);

  static std::string ingredientFindChanceLabel();
  static std::string ingredientFindChanceDescription();
  static std::string ingredientFindChanceValue(const CharacterDerivedStats& derived);

  static std::string foodConsumptionPerDayLabel();
  static std::string foodConsumptionPerDayDescription();
  static std::string foodConsumptionPerDayValue(const CharacterDerivedStats& derived);
};

} // namespace model
