#pragma once

#include "bmin/String.h"
namespace model {

struct CharacterStatDefinitions {
  static bmin::String attributesTitle();
  static bmin::String strengthLabel();
  static bmin::String strengthDescription();
  static bmin::String agilityLabel();
  static bmin::String agilityDescription();
  static bmin::String constitutionLabel();
  static bmin::String constitutionDescription();
  static bmin::String mindLabel();
  static bmin::String mindDescription();
  static bmin::String luckLabel();
  static bmin::String luckDescription();

  static bmin::String weaponMasteryTitle();
  static bmin::String edgedWeaponsLabel();
  static bmin::String edgedWeaponsDescription();
  static bmin::String poleWeaponsLabel();
  static bmin::String poleWeaponsDescription();
  static bmin::String bluntWeaponsLabel();
  static bmin::String bluntWeaponsDescription();
  static bmin::String rangeWeaponsLabel();
  static bmin::String rangeWeaponsDescription();
  static bmin::String unarmedLabel();
  static bmin::String unarmedDescription();

  static bmin::String magicMasteryTitle();
  static bmin::String manaLabel();
  static bmin::String manaDescription();
  static bmin::String abilityPowerLabel();
  static bmin::String abilityPowerDescription();
  static bmin::String attunementLabel();
  static bmin::String attunementDescription();
  static bmin::String faithLabel();
  static bmin::String faithDescription();
  static bmin::String loreLabel();
  static bmin::String loreDescription();

  static bmin::String bodyMasteryTitle();
  static bmin::String resistPhysicalLabel();
  static bmin::String resistPhysicalDescription();
  static bmin::String resistMagicalLabel();
  static bmin::String resistMagicalDescription();
  static bmin::String healingEffLabel();
  static bmin::String healingEffDescription();
  static bmin::String damageReductionLabel();
  static bmin::String damageReductionDescription();
  static bmin::String armorTrainingLabel();
  static bmin::String armorTrainingDescription();

  static bmin::String skillsTitle();
  static bmin::String trickeryLabel();
  static bmin::String trickeryDescription();
  static bmin::String stealthLabel();
  static bmin::String stealthDescription();
  static bmin::String socialLabel();
  static bmin::String socialDescription();
  static bmin::String magicItemUseLabel();
  static bmin::String magicItemUseDescription();
  static bmin::String cookingLabel();
  static bmin::String cookingDescription();
  static bmin::String acrobaticsLabel();
  static bmin::String acrobaticsDescription();
  static bmin::String survivalLabel();
  static bmin::String survivalDescription();
  static bmin::String focusLabel();
  static bmin::String focusDescription();
  static bmin::String conditioningLabel();
  static bmin::String conditioningDescription();
};

} // namespace model
