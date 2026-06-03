#include "model/stats/CharacterStatDefinitions.h"
#include "lib/sdl2w/L10n.h"

namespace model {

std::string CharacterStatDefinitions::attributesTitle() { return TRANSLATE("Attributes"); }

std::string CharacterStatDefinitions::strengthLabel() { return TRANSLATE("Strength"); }
std::string CharacterStatDefinitions::strengthDescription() {
  return TRANSLATE("Each point grants 1 additional base damage to melee weapons.");
}
std::string CharacterStatDefinitions::agilityLabel() { return TRANSLATE("Agility"); }
std::string CharacterStatDefinitions::agilityDescription() {
  return TRANSLATE("Each point reduces cost of actions by 1 (to a minimum of 1).");
}
std::string CharacterStatDefinitions::constitutionLabel() {
  return TRANSLATE("Constitution");
}
std::string CharacterStatDefinitions::constitutionDescription() {
  return TRANSLATE("Each point grants 5 HP per level.");
}
std::string CharacterStatDefinitions::mindLabel() { return TRANSLATE("Mind"); }
std::string CharacterStatDefinitions::mindDescription() {
  return TRANSLATE("Each point grants 1 additional base damage to magic spells.");
}
std::string CharacterStatDefinitions::luckLabel() { return TRANSLATE("Luck"); }
std::string CharacterStatDefinitions::luckDescription() {
  return TRANSLATE("Each point raises the minimum of every combat dice roll by 1d2 - 1.");
}

std::string CharacterStatDefinitions::weaponMasteryTitle() {
  return TRANSLATE("Weapon Mastery");
}
std::string CharacterStatDefinitions::edgedWeaponsLabel() { return TRANSLATE("Edged Weapons"); }
std::string CharacterStatDefinitions::edgedWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. One in each hand; high hit rate, B damage.");
}
std::string CharacterStatDefinitions::poleWeaponsLabel() { return TRANSLATE("Pole Weapons"); }
std::string CharacterStatDefinitions::poleWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Two-handed; lower hit rate, A damage.");
}
std::string CharacterStatDefinitions::bluntWeaponsLabel() { return TRANSLATE("Blunt Weapons"); }
std::string CharacterStatDefinitions::bluntWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. One or two handed; high hit rate, damage range "
      "C-A.");
}
std::string CharacterStatDefinitions::rangeWeaponsLabel() {
  return TRANSLATE("Range Weapons");
}
std::string CharacterStatDefinitions::rangeWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Simple ranged weapons scale linearly; complex "
      "ranged weapons scale exponentially.");
}
std::string CharacterStatDefinitions::unarmedLabel() { return TRANSLATE("Unarmed"); }
std::string CharacterStatDefinitions::unarmedDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Can chain many hits; high hit rate, C damage.");
}

std::string CharacterStatDefinitions::magicMasteryTitle() { return TRANSLATE("Magic Mastery"); }
std::string CharacterStatDefinitions::manaLabel() { return TRANSLATE("Mana"); }
std::string CharacterStatDefinitions::manaDescription() {
  return TRANSLATE("Flat increases mana by 1 point.");
}
std::string CharacterStatDefinitions::abilityPowerLabel() {
  return TRANSLATE("Ability Power");
}
std::string CharacterStatDefinitions::abilityPowerDescription() {
  return TRANSLATE("Spells use this stat to base their damage on, usually as a ratio.");
}
std::string CharacterStatDefinitions::attunementLabel() { return TRANSLATE("Attunement"); }
std::string CharacterStatDefinitions::attunementDescription() {
  return TRANSLATE(
      "Magical items require an attunement level to use. Increases effectiveness of one-time "
      "magic items (scrolls, potions, etc.).");
}
std::string CharacterStatDefinitions::faithLabel() { return TRANSLATE("Faith"); }
std::string CharacterStatDefinitions::faithDescription() {
  return TRANSLATE("Reduces length of bad status effects by 1 turn.");
}
std::string CharacterStatDefinitions::loreLabel() { return TRANSLATE("Lore"); }
std::string CharacterStatDefinitions::loreDescription() {
  return TRANSLATE("Percentage-based magic damage increase.");
}

std::string CharacterStatDefinitions::bodyMasteryTitle() { return TRANSLATE("Body Mastery"); }
std::string CharacterStatDefinitions::resistPhysicalLabel() {
  return TRANSLATE("Resist Physical");
}
std::string CharacterStatDefinitions::resistPhysicalDescription() {
  return TRANSLATE("Reduces physical damage by 5% per point.");
}
std::string CharacterStatDefinitions::resistMagicalLabel() {
  return TRANSLATE("Resist Magical");
}
std::string CharacterStatDefinitions::resistMagicalDescription() {
  return TRANSLATE("Reduces magical damage by 5% per point.");
}
std::string CharacterStatDefinitions::healingEffLabel() { return TRANSLATE("Healing Eff."); }
std::string CharacterStatDefinitions::healingEffDescription() {
  return TRANSLATE("Each point raises amount healed from outside sources by 1.");
}
std::string CharacterStatDefinitions::damageReductionLabel() {
  return TRANSLATE("Damage Reduction");
}
std::string CharacterStatDefinitions::damageReductionDescription() {
  return TRANSLATE("Flat damage reduction all around.");
}
std::string CharacterStatDefinitions::armorTrainingLabel() {
  return TRANSLATE("Armor Training");
}
std::string CharacterStatDefinitions::armorTrainingDescription() {
  return TRANSLATE(
      "Certain armor requires armor training level. Better training reduces movement cost "
      "when wearing it.");
}

std::string CharacterStatDefinitions::skillsTitle() { return TRANSLATE("Skills"); }
std::string CharacterStatDefinitions::trickeryLabel() { return TRANSLATE("Trickery"); }
std::string CharacterStatDefinitions::trickeryDescription() {
  return TRANSLATE(
      "Each level grants 1 less tool required to open locks. Every 5th level grants a bonus "
      "to sleight of hand checks (stealing, dialogue).");
}
std::string CharacterStatDefinitions::stealthLabel() { return TRANSLATE("Stealth"); }
std::string CharacterStatDefinitions::stealthDescription() {
  return TRANSLATE(
      "Each level increases time before you are noticed while in stealth. Every 5th level "
      "decreases enemies' vision radius on you by one.");
}
std::string CharacterStatDefinitions::socialLabel() { return TRANSLATE("Social"); }
std::string CharacterStatDefinitions::socialDescription() {
  return TRANSLATE(
      "Each level grants 1% discount on buying trade goods. Every 5th level grants a 10% "
      "boost to disposition toward you.");
}
std::string CharacterStatDefinitions::magicItemUseLabel() {
  return TRANSLATE("Magic Item Use");
}
std::string CharacterStatDefinitions::magicItemUseDescription() {
  return TRANSLATE(
      "Each level grants 1 higher threshold for learning arcane knowledge from runes. Every "
      "5th level doubles potency of magical items (potions, wands).");
}
std::string CharacterStatDefinitions::cookingLabel() { return TRANSLATE("Cooking"); }
std::string CharacterStatDefinitions::cookingDescription() {
  return TRANSLATE(
      "Each level reduces food required to be satiated while traveling by 1. Every 5th level "
      "allows cooking better food with greater benefits.");
}
std::string CharacterStatDefinitions::acrobaticsLabel() { return TRANSLATE("Acrobatics"); }
std::string CharacterStatDefinitions::acrobaticsDescription() {
  return TRANSLATE(
      "Each level reduces time to pass through overworld tiles by 1 tick. Every 5th level "
      "allows moving an extra tile before costing AP.");
}
std::string CharacterStatDefinitions::survivalLabel() { return TRANSLATE("Survival"); }
std::string CharacterStatDefinitions::survivalDescription() {
  return TRANSLATE(
      "Each level grants 1 additional roll for finding alchemy or cooking ingredients when a "
      "day ends while traveling. Every 5th level grants 1d4 per level higher health and mana "
      "after fights.");
}
std::string CharacterStatDefinitions::focusLabel() { return TRANSLATE("Focus"); }
std::string CharacterStatDefinitions::focusDescription() {
  return TRANSLATE(
      "Each level grants 4 additional mana. Every 5th level grants 1 additional point to your "
      "highest combat trainable skill.");
}
std::string CharacterStatDefinitions::conditioningLabel() { return TRANSLATE("Conditioning"); }
std::string CharacterStatDefinitions::conditioningDescription() {
  return TRANSLATE(
      "Each level grants 1 additional HP when using a healing potion or item. Every 5th level "
      "grants +1 to all combat stats.");
}

} // namespace model
