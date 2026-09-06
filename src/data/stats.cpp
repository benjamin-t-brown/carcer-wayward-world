module;
#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <variant>

module carcer.data;

import bmin.string_interop;
import sdl2w;

#include "macros.h"

// --- stats/CharacterDerivedStats.cppm ---

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
// --- stats/CharacterDerivedStatDefinitions.cppm ---

namespace model {

namespace {

bmin::String percentValue(int value) { return bmin::toString(value) + "%"; }

} // namespace

bmin::String CharacterDerivedStatDefinitions::derivedTitle() { return TRANSLATE("Derived"); }
bmin::String CharacterDerivedStatDefinitions::derivedSkillsTitle() {
  return TRANSLATE("Derived Skills");
}

bmin::String CharacterDerivedStatDefinitions::actionPointsLabel() {
  return TRANSLATE("Action Points");
}
bmin::String CharacterDerivedStatDefinitions::actionPointsDescription() {
  return TRANSLATE("Base 1.");
}
bmin::String CharacterDerivedStatDefinitions::actionPointsValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.actionPoints);
}

bmin::String CharacterDerivedStatDefinitions::mightDamageLabel() {
  return TRANSLATE("Might/Damage");
}
bmin::String CharacterDerivedStatDefinitions::mightDamageDescription() {
  return TRANSLATE("Base 1, plus Strength.");
}
bmin::String CharacterDerivedStatDefinitions::mightDamageValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.mightDamage);
}

bmin::String CharacterDerivedStatDefinitions::magicDamageLabel() {
  return TRANSLATE("Magic/Damage");
}
bmin::String CharacterDerivedStatDefinitions::magicDamageDescription() {
  return TRANSLATE("Base 0, plus Mind.");
}
bmin::String CharacterDerivedStatDefinitions::magicDamageValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.magicDamage);
}

bmin::String CharacterDerivedStatDefinitions::hpLabel() { return TRANSLATE("HP"); }
bmin::String CharacterDerivedStatDefinitions::hpDescription() {
  return TRANSLATE("Base 10, plus 5 per Constitution.");
}
bmin::String CharacterDerivedStatDefinitions::hpValue(const CharacterDerivedStats& derived) {
  return bmin::toString(derived.hp);
}

bmin::String CharacterDerivedStatDefinitions::attackHitChanceLabel() {
  return TRANSLATE("Attack Hit Chance");
}
bmin::String CharacterDerivedStatDefinitions::attackHitChanceDescription() {
  return TRANSLATE("Base 50%.");
}
bmin::String CharacterDerivedStatDefinitions::attackHitChanceValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.attackHitChancePercent);
}

bmin::String CharacterDerivedStatDefinitions::abilityPowerLabel() {
  return TRANSLATE("Ability Power");
}
bmin::String CharacterDerivedStatDefinitions::abilityPowerDescription() {
  return TRANSLATE("Base 0, from Magic Mastery training.");
}
bmin::String CharacterDerivedStatDefinitions::abilityPowerValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.abilityPower);
}

bmin::String CharacterDerivedStatDefinitions::damageReductionLabel() {
  return TRANSLATE("Damage Reduction");
}
bmin::String CharacterDerivedStatDefinitions::damageReductionDescription() {
  return TRANSLATE("Base 0, from Body Mastery training.");
}
bmin::String CharacterDerivedStatDefinitions::damageReductionValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.damageReduction);
}

bmin::String CharacterDerivedStatDefinitions::armorClassLabel() {
  return TRANSLATE("Armor Class");
}
bmin::String CharacterDerivedStatDefinitions::armorClassDescription() {
  return TRANSLATE("Base 10.");
}
bmin::String CharacterDerivedStatDefinitions::armorClassValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.armorClass);
}

bmin::String CharacterDerivedStatDefinitions::spellPotencyLabel() {
  return TRANSLATE("Spell Potency");
}
bmin::String CharacterDerivedStatDefinitions::spellPotencyDescription() {
  return TRANSLATE("Spell potency, status effect length, and field length. Base 1.");
}
bmin::String CharacterDerivedStatDefinitions::spellPotencyValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.spellPotency);
}

bmin::String CharacterDerivedStatDefinitions::manaLabel() { return TRANSLATE("Mana"); }
bmin::String CharacterDerivedStatDefinitions::manaDescription() {
  return TRANSLATE("Base 10, plus Magic Mastery and Focus.");
}
bmin::String CharacterDerivedStatDefinitions::manaValue(const CharacterDerivedStats& derived) {
  return bmin::toString(derived.maxMana);
}

bmin::String CharacterDerivedStatDefinitions::resistancesLabel() {
  return TRANSLATE("Resistances");
}
bmin::String CharacterDerivedStatDefinitions::resistancesDescription() {
  return TRANSLATE("Base none.");
}
bmin::String CharacterDerivedStatDefinitions::resistancesValue(
    const CharacterDerivedStats&) {
  return TRANSLATE("None");
}

bmin::String CharacterDerivedStatDefinitions::jumpDistanceLabel() {
  return TRANSLATE("Jump Distance");
}
bmin::String CharacterDerivedStatDefinitions::jumpDistanceDescription() {
  return TRANSLATE("Base 2.");
}
bmin::String CharacterDerivedStatDefinitions::jumpDistanceValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.jumpDistance);
}

bmin::String CharacterDerivedStatDefinitions::healingEffectivenessLabel() {
  return TRANSLATE("Healing Effectiveness");
}
bmin::String CharacterDerivedStatDefinitions::healingEffectivenessDescription() {
  return TRANSLATE("Base 100%.");
}
bmin::String CharacterDerivedStatDefinitions::healingEffectivenessValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.healingEffectivenessPercent);
}

bmin::String CharacterDerivedStatDefinitions::statusEffectShieldLabel() {
  return TRANSLATE("Status Effect Shield");
}
bmin::String CharacterDerivedStatDefinitions::statusEffectShieldDescription() {
  return TRANSLATE("Base 0.");
}
bmin::String CharacterDerivedStatDefinitions::statusEffectShieldValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.statusEffectShield);
}

bmin::String CharacterDerivedStatDefinitions::materiaSlotsLabel() {
  return TRANSLATE("Materia Slots");
}
bmin::String CharacterDerivedStatDefinitions::materiaSlotsDescription() {
  return TRANSLATE("Base 0.");
}
bmin::String CharacterDerivedStatDefinitions::materiaSlotsValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.materiaSlots);
}

bmin::String CharacterDerivedStatDefinitions::shieldBonusLabel() {
  return TRANSLATE("Shield Bonus");
}
bmin::String CharacterDerivedStatDefinitions::shieldBonusDescription() {
  return TRANSLATE("Base 0.");
}
bmin::String CharacterDerivedStatDefinitions::shieldBonusValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.shieldBonus);
}

bmin::String CharacterDerivedStatDefinitions::enemyVisionRangeLabel() {
  return TRANSLATE("Enemy Vision Range");
}
bmin::String CharacterDerivedStatDefinitions::enemyVisionRangeDescription() {
  return TRANSLATE("Affected by Stealth training.");
}
bmin::String CharacterDerivedStatDefinitions::enemyVisionRangeValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.enemyVisionRange);
}

bmin::String CharacterDerivedStatDefinitions::mageLoreLabel() {
  return TRANSLATE("Mage Lore");
}
bmin::String CharacterDerivedStatDefinitions::mageLoreDescription() {
  return TRANSLATE("Affected by Magic Item Use training.");
}
bmin::String CharacterDerivedStatDefinitions::mageLoreValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.mageLore);
}

bmin::String CharacterDerivedStatDefinitions::toolUseLabel() { return TRANSLATE("Tool Use"); }
bmin::String CharacterDerivedStatDefinitions::toolUseDescription() {
  return TRANSLATE("Affected by Trickery training.");
}
bmin::String CharacterDerivedStatDefinitions::toolUseValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.toolUse);
}

bmin::String CharacterDerivedStatDefinitions::tradeDiscountLabel() {
  return TRANSLATE("Trade Discount");
}
bmin::String CharacterDerivedStatDefinitions::tradeDiscountDescription() {
  return TRANSLATE("Base 0%, plus 1% per Social level.");
}
bmin::String CharacterDerivedStatDefinitions::tradeDiscountValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.tradeDiscountPercent);
}

bmin::String CharacterDerivedStatDefinitions::itemUsageLabel() {
  return TRANSLATE("Item Usage");
}
bmin::String CharacterDerivedStatDefinitions::itemUsageDescription() {
  return TRANSLATE("Base 100%.");
}
bmin::String CharacterDerivedStatDefinitions::itemUsageValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.itemUsagePercent);
}

bmin::String CharacterDerivedStatDefinitions::foodConsumptionLabel() {
  return TRANSLATE("Food Consumption");
}
bmin::String CharacterDerivedStatDefinitions::foodConsumptionDescription() {
  return TRANSLATE("Base 100, reduced by Cooking training.");
}
bmin::String CharacterDerivedStatDefinitions::foodConsumptionValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.foodConsumption);
}

bmin::String CharacterDerivedStatDefinitions::firstAidLabel() { return TRANSLATE("First Aid"); }
bmin::String CharacterDerivedStatDefinitions::firstAidDescription() {
  return TRANSLATE("Base 1d4 per level.");
}
bmin::String CharacterDerivedStatDefinitions::firstAidValue(
    const CharacterDerivedStats& derived) {
  return "1d4 x " + bmin::toString(derived.firstAidPerLevel);
}

bmin::String CharacterDerivedStatDefinitions::ingredientFindChanceLabel() {
  return TRANSLATE("Ingredient Find Chance");
}
bmin::String CharacterDerivedStatDefinitions::ingredientFindChanceDescription() {
  return TRANSLATE("Base 10%, plus Survival training.");
}
bmin::String CharacterDerivedStatDefinitions::ingredientFindChanceValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.ingredientFindChancePercent);
}

bmin::String CharacterDerivedStatDefinitions::foodConsumptionPerDayLabel() {
  return TRANSLATE("Food Consumption Per Day");
}
bmin::String CharacterDerivedStatDefinitions::foodConsumptionPerDayDescription() {
  return TRANSLATE("Base 100.");
}
bmin::String CharacterDerivedStatDefinitions::foodConsumptionPerDayValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.foodConsumptionPerDay);
}

} // namespace model

// --- stats/CharacterStatDefinitions.cppm ---

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
