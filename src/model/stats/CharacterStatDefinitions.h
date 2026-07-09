#pragma once

#include "lib/Types.h"

namespace model {

struct CharacterStatDefinitions {
  static String attributesTitle();
  static String strengthLabel();
  static String strengthDescription();
  static String agilityLabel();
  static String agilityDescription();
  static String constitutionLabel();
  static String constitutionDescription();
  static String mindLabel();
  static String mindDescription();
  static String luckLabel();
  static String luckDescription();

  static String weaponMasteryTitle();
  static String edgedWeaponsLabel();
  static String edgedWeaponsDescription();
  static String poleWeaponsLabel();
  static String poleWeaponsDescription();
  static String bluntWeaponsLabel();
  static String bluntWeaponsDescription();
  static String rangeWeaponsLabel();
  static String rangeWeaponsDescription();
  static String unarmedLabel();
  static String unarmedDescription();

  static String magicMasteryTitle();
  static String manaLabel();
  static String manaDescription();
  static String abilityPowerLabel();
  static String abilityPowerDescription();
  static String attunementLabel();
  static String attunementDescription();
  static String faithLabel();
  static String faithDescription();
  static String loreLabel();
  static String loreDescription();

  static String bodyMasteryTitle();
  static String resistPhysicalLabel();
  static String resistPhysicalDescription();
  static String resistMagicalLabel();
  static String resistMagicalDescription();
  static String healingEffLabel();
  static String healingEffDescription();
  static String damageReductionLabel();
  static String damageReductionDescription();
  static String armorTrainingLabel();
  static String armorTrainingDescription();

  static String skillsTitle();
  static String trickeryLabel();
  static String trickeryDescription();
  static String stealthLabel();
  static String stealthDescription();
  static String socialLabel();
  static String socialDescription();
  static String magicItemUseLabel();
  static String magicItemUseDescription();
  static String cookingLabel();
  static String cookingDescription();
  static String acrobaticsLabel();
  static String acrobaticsDescription();
  static String survivalLabel();
  static String survivalDescription();
  static String focusLabel();
  static String focusDescription();
  static String conditioningLabel();
  static String conditioningDescription();
};

} // namespace model
