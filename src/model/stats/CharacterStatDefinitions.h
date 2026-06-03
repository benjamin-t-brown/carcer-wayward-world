#pragma once

#include <string>

namespace model {

struct CharacterStatDefinitions {
  static std::string attributesTitle();
  static std::string strengthLabel();
  static std::string strengthDescription();
  static std::string agilityLabel();
  static std::string agilityDescription();
  static std::string constitutionLabel();
  static std::string constitutionDescription();
  static std::string mindLabel();
  static std::string mindDescription();
  static std::string luckLabel();
  static std::string luckDescription();

  static std::string weaponMasteryTitle();
  static std::string edgedWeaponsLabel();
  static std::string edgedWeaponsDescription();
  static std::string poleWeaponsLabel();
  static std::string poleWeaponsDescription();
  static std::string bluntWeaponsLabel();
  static std::string bluntWeaponsDescription();
  static std::string rangeWeaponsLabel();
  static std::string rangeWeaponsDescription();
  static std::string unarmedLabel();
  static std::string unarmedDescription();

  static std::string magicMasteryTitle();
  static std::string manaLabel();
  static std::string manaDescription();
  static std::string abilityPowerLabel();
  static std::string abilityPowerDescription();
  static std::string attunementLabel();
  static std::string attunementDescription();
  static std::string faithLabel();
  static std::string faithDescription();
  static std::string loreLabel();
  static std::string loreDescription();

  static std::string bodyMasteryTitle();
  static std::string resistPhysicalLabel();
  static std::string resistPhysicalDescription();
  static std::string resistMagicalLabel();
  static std::string resistMagicalDescription();
  static std::string healingEffLabel();
  static std::string healingEffDescription();
  static std::string damageReductionLabel();
  static std::string damageReductionDescription();
  static std::string armorTrainingLabel();
  static std::string armorTrainingDescription();

  static std::string skillsTitle();
  static std::string trickeryLabel();
  static std::string trickeryDescription();
  static std::string stealthLabel();
  static std::string stealthDescription();
  static std::string socialLabel();
  static std::string socialDescription();
  static std::string magicItemUseLabel();
  static std::string magicItemUseDescription();
  static std::string cookingLabel();
  static std::string cookingDescription();
  static std::string acrobaticsLabel();
  static std::string acrobaticsDescription();
  static std::string survivalLabel();
  static std::string survivalDescription();
  static std::string focusLabel();
  static std::string focusDescription();
  static std::string conditioningLabel();
  static std::string conditioningDescription();
};

} // namespace model
