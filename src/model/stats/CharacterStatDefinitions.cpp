#include "model/stats/CharacterStatDefinitions.h"
#include "lib/sdl2w/L10n.h"

namespace model {

String CharacterStatDefinitions::attributesTitle() { return TRANSLATE("Attributes"); }

String CharacterStatDefinitions::strengthLabel() { return TRANSLATE("Strength"); }
String CharacterStatDefinitions::strengthDescription() {
  return TRANSLATE("Each point grants 1 additional base damage to melee weapons.");
}
String CharacterStatDefinitions::agilityLabel() { return TRANSLATE("Agility"); }
String CharacterStatDefinitions::agilityDescription() {
  return TRANSLATE("Each point reduces cost of actions by 1 (to a minimum of 1).");
}
String CharacterStatDefinitions::constitutionLabel() {
  return TRANSLATE("Constitution");
}
String CharacterStatDefinitions::constitutionDescription() {
  return TRANSLATE("Each point grants 5 HP per level.");
}
String CharacterStatDefinitions::mindLabel() { return TRANSLATE("Mind"); }
String CharacterStatDefinitions::mindDescription() {
  return TRANSLATE("Each point grants 1 additional base damage to magic spells.");
}
String CharacterStatDefinitions::luckLabel() { return TRANSLATE("Luck"); }
String CharacterStatDefinitions::luckDescription() {
  return TRANSLATE("Each point raises the minimum of every combat dice roll by 1d2 - 1.");
}

String CharacterStatDefinitions::weaponMasteryTitle() {
  return TRANSLATE("Weapon Mastery");
}
String CharacterStatDefinitions::edgedWeaponsLabel() { return TRANSLATE("Edged Weapons"); }
String CharacterStatDefinitions::edgedWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. One in each hand; high hit rate, B damage.");
}
String CharacterStatDefinitions::poleWeaponsLabel() { return TRANSLATE("Pole Weapons"); }
String CharacterStatDefinitions::poleWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Two-handed; lower hit rate, A damage.");
}
String CharacterStatDefinitions::bluntWeaponsLabel() { return TRANSLATE("Blunt Weapons"); }
String CharacterStatDefinitions::bluntWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. One or two handed; high hit rate, damage range "
      "C-A.");
}
String CharacterStatDefinitions::rangeWeaponsLabel() {
  return TRANSLATE("Range Weapons");
}
String CharacterStatDefinitions::rangeWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Simple ranged weapons scale linearly; complex "
      "ranged weapons scale exponentially.");
}
String CharacterStatDefinitions::unarmedLabel() { return TRANSLATE("Unarmed"); }
String CharacterStatDefinitions::unarmedDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Can chain many hits; high hit rate, C damage.");
}

String CharacterStatDefinitions::magicMasteryTitle() { return TRANSLATE("Magic Mastery"); }
String CharacterStatDefinitions::manaLabel() { return TRANSLATE("Mana"); }
String CharacterStatDefinitions::manaDescription() {
  return TRANSLATE("Flat increases mana by 1 point.");
}
String CharacterStatDefinitions::abilityPowerLabel() {
  return TRANSLATE("Ability Power");
}
String CharacterStatDefinitions::abilityPowerDescription() {
  return TRANSLATE("Spells use this stat to base their damage on, usually as a ratio.");
}
String CharacterStatDefinitions::attunementLabel() { return TRANSLATE("Attunement"); }
String CharacterStatDefinitions::attunementDescription() {
  return TRANSLATE(
      "Magical items require an attunement level to use. Increases effectiveness of one-time "
      "magic items (scrolls, potions, etc.).");
}
String CharacterStatDefinitions::faithLabel() { return TRANSLATE("Faith"); }
String CharacterStatDefinitions::faithDescription() {
  return TRANSLATE("Reduces length of bad status effects by 1 turn.");
}
String CharacterStatDefinitions::loreLabel() { return TRANSLATE("Lore"); }
String CharacterStatDefinitions::loreDescription() {
  return TRANSLATE("Percentage-based magic damage increase.");
}

String CharacterStatDefinitions::bodyMasteryTitle() { return TRANSLATE("Body Mastery"); }
String CharacterStatDefinitions::resistPhysicalLabel() {
  return TRANSLATE("Resist Physical");
}
String CharacterStatDefinitions::resistPhysicalDescription() {
  return TRANSLATE("Reduces physical damage by 5% per point.");
}
String CharacterStatDefinitions::resistMagicalLabel() {
  return TRANSLATE("Resist Magical");
}
String CharacterStatDefinitions::resistMagicalDescription() {
  return TRANSLATE("Reduces magical damage by 5% per point.");
}
String CharacterStatDefinitions::healingEffLabel() { return TRANSLATE("Healing Eff."); }
String CharacterStatDefinitions::healingEffDescription() {
  return TRANSLATE("Each point raises amount healed from outside sources by 1.");
}
String CharacterStatDefinitions::damageReductionLabel() {
  return TRANSLATE("Damage Reduction");
}
String CharacterStatDefinitions::damageReductionDescription() {
  return TRANSLATE("Flat damage reduction all around.");
}
String CharacterStatDefinitions::armorTrainingLabel() {
  return TRANSLATE("Armor Training");
}
String CharacterStatDefinitions::armorTrainingDescription() {
  return TRANSLATE(
      "Certain armor requires armor training level. Better training reduces movement cost "
      "when wearing it.");
}

String CharacterStatDefinitions::skillsTitle() { return TRANSLATE("Skills"); }
String CharacterStatDefinitions::trickeryLabel() { return TRANSLATE("Trickery"); }
String CharacterStatDefinitions::trickeryDescription() {
  return TRANSLATE(
      "Each level grants 1 less tool required to open locks. Every 5th level grants a bonus "
      "to sleight of hand checks (stealing, dialogue).");
}
String CharacterStatDefinitions::stealthLabel() { return TRANSLATE("Stealth"); }
String CharacterStatDefinitions::stealthDescription() {
  return TRANSLATE(
      "Each level increases time before you are noticed while in stealth. Every 5th level "
      "decreases enemies' vision radius on you by one.");
}
String CharacterStatDefinitions::socialLabel() { return TRANSLATE("Social"); }
String CharacterStatDefinitions::socialDescription() {
  return TRANSLATE(
      "Each level grants 1% discount on buying trade goods. Every 5th level grants a 10% "
      "boost to disposition toward you.");
}
String CharacterStatDefinitions::magicItemUseLabel() {
  return TRANSLATE("Magic Item Use");
}
String CharacterStatDefinitions::magicItemUseDescription() {
  return TRANSLATE(
      "Each level grants 1 higher threshold for learning arcane knowledge from runes. Every "
      "5th level doubles potency of magical items (potions, wands).");
}
String CharacterStatDefinitions::cookingLabel() { return TRANSLATE("Cooking"); }
String CharacterStatDefinitions::cookingDescription() {
  return TRANSLATE(
      "Each level reduces food required to be satiated while traveling by 1. Every 5th level "
      "allows cooking better food with greater benefits.");
}
String CharacterStatDefinitions::acrobaticsLabel() { return TRANSLATE("Acrobatics"); }
String CharacterStatDefinitions::acrobaticsDescription() {
  return TRANSLATE(
      "Each level reduces time to pass through overworld tiles by 1 tick. Every 5th level "
      "allows moving an extra tile before costing AP.");
}
String CharacterStatDefinitions::survivalLabel() { return TRANSLATE("Survival"); }
String CharacterStatDefinitions::survivalDescription() {
  return TRANSLATE(
      "Each level grants 1 additional roll for finding alchemy or cooking ingredients when a "
      "day ends while traveling. Every 5th level grants 1d4 per level higher health and mana "
      "after fights.");
}
String CharacterStatDefinitions::focusLabel() { return TRANSLATE("Focus"); }
String CharacterStatDefinitions::focusDescription() {
  return TRANSLATE(
      "Each level grants 4 additional mana. Every 5th level grants 1 additional point to your "
      "highest combat trainable skill.");
}
String CharacterStatDefinitions::conditioningLabel() { return TRANSLATE("Conditioning"); }
String CharacterStatDefinitions::conditioningDescription() {
  return TRANSLATE(
      "Each level grants 1 additional HP when using a healing potion or item. Every 5th level "
      "grants +1 to all combat stats.");
}

} // namespace model
