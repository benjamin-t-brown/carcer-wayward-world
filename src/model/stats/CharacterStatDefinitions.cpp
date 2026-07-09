#include "model/stats/CharacterStatDefinitions.h"
#include "sdl2w/L10n.h"

namespace model {

bmin::String CharacterStatDefinitions::attributesTitle() { return TRANSLATE("Attributes"); }

bmin::String CharacterStatDefinitions::strengthLabel() { return TRANSLATE("Strength"); }
bmin::String CharacterStatDefinitions::strengthDescription() {
  return TRANSLATE("Each point grants 1 additional base damage to melee weapons.");
}
bmin::String CharacterStatDefinitions::agilityLabel() { return TRANSLATE("Agility"); }
bmin::String CharacterStatDefinitions::agilityDescription() {
  return TRANSLATE("Each point reduces cost of actions by 1 (to a minimum of 1).");
}
bmin::String CharacterStatDefinitions::constitutionLabel() {
  return TRANSLATE("Constitution");
}
bmin::String CharacterStatDefinitions::constitutionDescription() {
  return TRANSLATE("Each point grants 5 HP per level.");
}
bmin::String CharacterStatDefinitions::mindLabel() { return TRANSLATE("Mind"); }
bmin::String CharacterStatDefinitions::mindDescription() {
  return TRANSLATE("Each point grants 1 additional base damage to magic spells.");
}
bmin::String CharacterStatDefinitions::luckLabel() { return TRANSLATE("Luck"); }
bmin::String CharacterStatDefinitions::luckDescription() {
  return TRANSLATE("Each point raises the minimum of every combat dice roll by 1d2 - 1.");
}

bmin::String CharacterStatDefinitions::weaponMasteryTitle() {
  return TRANSLATE("Weapon Mastery");
}
bmin::String CharacterStatDefinitions::edgedWeaponsLabel() { return TRANSLATE("Edged Weapons"); }
bmin::String CharacterStatDefinitions::edgedWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. One in each hand; high hit rate, B damage.");
}
bmin::String CharacterStatDefinitions::poleWeaponsLabel() { return TRANSLATE("Pole Weapons"); }
bmin::String CharacterStatDefinitions::poleWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Two-handed; lower hit rate, A damage.");
}
bmin::String CharacterStatDefinitions::bluntWeaponsLabel() { return TRANSLATE("Blunt Weapons"); }
bmin::String CharacterStatDefinitions::bluntWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. One or two handed; high hit rate, damage range "
      "C-A.");
}
bmin::String CharacterStatDefinitions::rangeWeaponsLabel() {
  return TRANSLATE("Range Weapons");
}
bmin::String CharacterStatDefinitions::rangeWeaponsDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Simple ranged weapons scale linearly; complex "
      "ranged weapons scale exponentially.");
}
bmin::String CharacterStatDefinitions::unarmedLabel() { return TRANSLATE("Unarmed"); }
bmin::String CharacterStatDefinitions::unarmedDescription() {
  return TRANSLATE(
      "Each point grants +1 to attack roll. Can chain many hits; high hit rate, C damage.");
}

bmin::String CharacterStatDefinitions::magicMasteryTitle() { return TRANSLATE("Magic Mastery"); }
bmin::String CharacterStatDefinitions::manaLabel() { return TRANSLATE("Mana"); }
bmin::String CharacterStatDefinitions::manaDescription() {
  return TRANSLATE("Flat increases mana by 1 point.");
}
bmin::String CharacterStatDefinitions::abilityPowerLabel() {
  return TRANSLATE("Ability Power");
}
bmin::String CharacterStatDefinitions::abilityPowerDescription() {
  return TRANSLATE("Spells use this stat to base their damage on, usually as a ratio.");
}
bmin::String CharacterStatDefinitions::attunementLabel() { return TRANSLATE("Attunement"); }
bmin::String CharacterStatDefinitions::attunementDescription() {
  return TRANSLATE(
      "Magical items require an attunement level to use. Increases effectiveness of one-time "
      "magic items (scrolls, potions, etc.).");
}
bmin::String CharacterStatDefinitions::faithLabel() { return TRANSLATE("Faith"); }
bmin::String CharacterStatDefinitions::faithDescription() {
  return TRANSLATE("Reduces length of bad status effects by 1 turn.");
}
bmin::String CharacterStatDefinitions::loreLabel() { return TRANSLATE("Lore"); }
bmin::String CharacterStatDefinitions::loreDescription() {
  return TRANSLATE("Percentage-based magic damage increase.");
}

bmin::String CharacterStatDefinitions::bodyMasteryTitle() { return TRANSLATE("Body Mastery"); }
bmin::String CharacterStatDefinitions::resistPhysicalLabel() {
  return TRANSLATE("Resist Physical");
}
bmin::String CharacterStatDefinitions::resistPhysicalDescription() {
  return TRANSLATE("Reduces physical damage by 5% per point.");
}
bmin::String CharacterStatDefinitions::resistMagicalLabel() {
  return TRANSLATE("Resist Magical");
}
bmin::String CharacterStatDefinitions::resistMagicalDescription() {
  return TRANSLATE("Reduces magical damage by 5% per point.");
}
bmin::String CharacterStatDefinitions::healingEffLabel() { return TRANSLATE("Healing Eff."); }
bmin::String CharacterStatDefinitions::healingEffDescription() {
  return TRANSLATE("Each point raises amount healed from outside sources by 1.");
}
bmin::String CharacterStatDefinitions::damageReductionLabel() {
  return TRANSLATE("Damage Reduction");
}
bmin::String CharacterStatDefinitions::damageReductionDescription() {
  return TRANSLATE("Flat damage reduction all around.");
}
bmin::String CharacterStatDefinitions::armorTrainingLabel() {
  return TRANSLATE("Armor Training");
}
bmin::String CharacterStatDefinitions::armorTrainingDescription() {
  return TRANSLATE(
      "Certain armor requires armor training level. Better training reduces movement cost "
      "when wearing it.");
}

bmin::String CharacterStatDefinitions::skillsTitle() { return TRANSLATE("Skills"); }
bmin::String CharacterStatDefinitions::trickeryLabel() { return TRANSLATE("Trickery"); }
bmin::String CharacterStatDefinitions::trickeryDescription() {
  return TRANSLATE(
      "Each level grants 1 less tool required to open locks. Every 5th level grants a bonus "
      "to sleight of hand checks (stealing, dialogue).");
}
bmin::String CharacterStatDefinitions::stealthLabel() { return TRANSLATE("Stealth"); }
bmin::String CharacterStatDefinitions::stealthDescription() {
  return TRANSLATE(
      "Each level increases time before you are noticed while in stealth. Every 5th level "
      "decreases enemies' vision radius on you by one.");
}
bmin::String CharacterStatDefinitions::socialLabel() { return TRANSLATE("Social"); }
bmin::String CharacterStatDefinitions::socialDescription() {
  return TRANSLATE(
      "Each level grants 1% discount on buying trade goods. Every 5th level grants a 10% "
      "boost to disposition toward you.");
}
bmin::String CharacterStatDefinitions::magicItemUseLabel() {
  return TRANSLATE("Magic Item Use");
}
bmin::String CharacterStatDefinitions::magicItemUseDescription() {
  return TRANSLATE(
      "Each level grants 1 higher threshold for learning arcane knowledge from runes. Every "
      "5th level doubles potency of magical items (potions, wands).");
}
bmin::String CharacterStatDefinitions::cookingLabel() { return TRANSLATE("Cooking"); }
bmin::String CharacterStatDefinitions::cookingDescription() {
  return TRANSLATE(
      "Each level reduces food required to be satiated while traveling by 1. Every 5th level "
      "allows cooking better food with greater benefits.");
}
bmin::String CharacterStatDefinitions::acrobaticsLabel() { return TRANSLATE("Acrobatics"); }
bmin::String CharacterStatDefinitions::acrobaticsDescription() {
  return TRANSLATE(
      "Each level reduces time to pass through overworld tiles by 1 tick. Every 5th level "
      "allows moving an extra tile before costing AP.");
}
bmin::String CharacterStatDefinitions::survivalLabel() { return TRANSLATE("Survival"); }
bmin::String CharacterStatDefinitions::survivalDescription() {
  return TRANSLATE(
      "Each level grants 1 additional roll for finding alchemy or cooking ingredients when a "
      "day ends while traveling. Every 5th level grants 1d4 per level higher health and mana "
      "after fights.");
}
bmin::String CharacterStatDefinitions::focusLabel() { return TRANSLATE("Focus"); }
bmin::String CharacterStatDefinitions::focusDescription() {
  return TRANSLATE(
      "Each level grants 4 additional mana. Every 5th level grants 1 additional point to your "
      "highest combat trainable skill.");
}
bmin::String CharacterStatDefinitions::conditioningLabel() { return TRANSLATE("Conditioning"); }
bmin::String CharacterStatDefinitions::conditioningDescription() {
  return TRANSLATE(
      "Each level grants 1 additional HP when using a healing potion or item. Every 5th level "
      "grants +1 to all combat stats.");
}

} // namespace model
