#include "model/stats/CharacterDerivedStatDefinitions.h"
#include "lib/sdl2w/L10n.h"
#include <string>

namespace model {

namespace {

std::string percentValue(int value) { return std::to_string(value) + "%"; }

} // namespace

std::string CharacterDerivedStatDefinitions::derivedTitle() { return TRANSLATE("Derived"); }
std::string CharacterDerivedStatDefinitions::derivedSkillsTitle() {
  return TRANSLATE("Derived Skills");
}

std::string CharacterDerivedStatDefinitions::actionPointsLabel() {
  return TRANSLATE("Action Points");
}
std::string CharacterDerivedStatDefinitions::actionPointsDescription() {
  return TRANSLATE("Base 1.");
}
std::string CharacterDerivedStatDefinitions::actionPointsValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.actionPoints);
}

std::string CharacterDerivedStatDefinitions::mightDamageLabel() {
  return TRANSLATE("Might/Damage");
}
std::string CharacterDerivedStatDefinitions::mightDamageDescription() {
  return TRANSLATE("Base 1, plus Strength.");
}
std::string CharacterDerivedStatDefinitions::mightDamageValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.mightDamage);
}

std::string CharacterDerivedStatDefinitions::magicDamageLabel() {
  return TRANSLATE("Magic/Damage");
}
std::string CharacterDerivedStatDefinitions::magicDamageDescription() {
  return TRANSLATE("Base 0, plus Mind.");
}
std::string CharacterDerivedStatDefinitions::magicDamageValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.magicDamage);
}

std::string CharacterDerivedStatDefinitions::hpLabel() { return TRANSLATE("HP"); }
std::string CharacterDerivedStatDefinitions::hpDescription() {
  return TRANSLATE("Base 10, plus 5 per Constitution.");
}
std::string CharacterDerivedStatDefinitions::hpValue(const CharacterDerivedStats& derived) {
  return std::to_string(derived.hp);
}

std::string CharacterDerivedStatDefinitions::attackHitChanceLabel() {
  return TRANSLATE("Attack Hit Chance");
}
std::string CharacterDerivedStatDefinitions::attackHitChanceDescription() {
  return TRANSLATE("Base 50%.");
}
std::string CharacterDerivedStatDefinitions::attackHitChanceValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.attackHitChancePercent);
}

std::string CharacterDerivedStatDefinitions::abilityPowerLabel() {
  return TRANSLATE("Ability Power");
}
std::string CharacterDerivedStatDefinitions::abilityPowerDescription() {
  return TRANSLATE("Base 0, from Magic Mastery training.");
}
std::string CharacterDerivedStatDefinitions::abilityPowerValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.abilityPower);
}

std::string CharacterDerivedStatDefinitions::damageReductionLabel() {
  return TRANSLATE("Damage Reduction");
}
std::string CharacterDerivedStatDefinitions::damageReductionDescription() {
  return TRANSLATE("Base 0, from Body Mastery training.");
}
std::string CharacterDerivedStatDefinitions::damageReductionValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.damageReduction);
}

std::string CharacterDerivedStatDefinitions::armorClassLabel() {
  return TRANSLATE("Armor Class");
}
std::string CharacterDerivedStatDefinitions::armorClassDescription() {
  return TRANSLATE("Base 10.");
}
std::string CharacterDerivedStatDefinitions::armorClassValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.armorClass);
}

std::string CharacterDerivedStatDefinitions::spellPotencyLabel() {
  return TRANSLATE("Spell Potency");
}
std::string CharacterDerivedStatDefinitions::spellPotencyDescription() {
  return TRANSLATE("Spell potency, status effect length, and field length. Base 1.");
}
std::string CharacterDerivedStatDefinitions::spellPotencyValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.spellPotency);
}

std::string CharacterDerivedStatDefinitions::manaLabel() { return TRANSLATE("Mana"); }
std::string CharacterDerivedStatDefinitions::manaDescription() {
  return TRANSLATE("Base 10, plus Magic Mastery and Focus.");
}
std::string CharacterDerivedStatDefinitions::manaValue(const CharacterDerivedStats& derived) {
  return std::to_string(derived.maxMana);
}

std::string CharacterDerivedStatDefinitions::resistancesLabel() {
  return TRANSLATE("Resistances");
}
std::string CharacterDerivedStatDefinitions::resistancesDescription() {
  return TRANSLATE("Base none.");
}
std::string CharacterDerivedStatDefinitions::resistancesValue(
    const CharacterDerivedStats&) {
  return TRANSLATE("None");
}

std::string CharacterDerivedStatDefinitions::jumpDistanceLabel() {
  return TRANSLATE("Jump Distance");
}
std::string CharacterDerivedStatDefinitions::jumpDistanceDescription() {
  return TRANSLATE("Base 2.");
}
std::string CharacterDerivedStatDefinitions::jumpDistanceValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.jumpDistance);
}

std::string CharacterDerivedStatDefinitions::healingEffectivenessLabel() {
  return TRANSLATE("Healing Effectiveness");
}
std::string CharacterDerivedStatDefinitions::healingEffectivenessDescription() {
  return TRANSLATE("Base 100%.");
}
std::string CharacterDerivedStatDefinitions::healingEffectivenessValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.healingEffectivenessPercent);
}

std::string CharacterDerivedStatDefinitions::statusEffectShieldLabel() {
  return TRANSLATE("Status Effect Shield");
}
std::string CharacterDerivedStatDefinitions::statusEffectShieldDescription() {
  return TRANSLATE("Base 0.");
}
std::string CharacterDerivedStatDefinitions::statusEffectShieldValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.statusEffectShield);
}

std::string CharacterDerivedStatDefinitions::materiaSlotsLabel() {
  return TRANSLATE("Materia Slots");
}
std::string CharacterDerivedStatDefinitions::materiaSlotsDescription() {
  return TRANSLATE("Base 0.");
}
std::string CharacterDerivedStatDefinitions::materiaSlotsValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.materiaSlots);
}

std::string CharacterDerivedStatDefinitions::shieldBonusLabel() {
  return TRANSLATE("Shield Bonus");
}
std::string CharacterDerivedStatDefinitions::shieldBonusDescription() {
  return TRANSLATE("Base 0.");
}
std::string CharacterDerivedStatDefinitions::shieldBonusValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.shieldBonus);
}

std::string CharacterDerivedStatDefinitions::enemyVisionRangeLabel() {
  return TRANSLATE("Enemy Vision Range");
}
std::string CharacterDerivedStatDefinitions::enemyVisionRangeDescription() {
  return TRANSLATE("Affected by Stealth training.");
}
std::string CharacterDerivedStatDefinitions::enemyVisionRangeValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.enemyVisionRange);
}

std::string CharacterDerivedStatDefinitions::mageLoreLabel() {
  return TRANSLATE("Mage Lore");
}
std::string CharacterDerivedStatDefinitions::mageLoreDescription() {
  return TRANSLATE("Affected by Magic Item Use training.");
}
std::string CharacterDerivedStatDefinitions::mageLoreValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.mageLore);
}

std::string CharacterDerivedStatDefinitions::toolUseLabel() { return TRANSLATE("Tool Use"); }
std::string CharacterDerivedStatDefinitions::toolUseDescription() {
  return TRANSLATE("Affected by Trickery training.");
}
std::string CharacterDerivedStatDefinitions::toolUseValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.toolUse);
}

std::string CharacterDerivedStatDefinitions::tradeDiscountLabel() {
  return TRANSLATE("Trade Discount");
}
std::string CharacterDerivedStatDefinitions::tradeDiscountDescription() {
  return TRANSLATE("Base 0%, plus 1% per Social level.");
}
std::string CharacterDerivedStatDefinitions::tradeDiscountValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.tradeDiscountPercent);
}

std::string CharacterDerivedStatDefinitions::itemUsageLabel() {
  return TRANSLATE("Item Usage");
}
std::string CharacterDerivedStatDefinitions::itemUsageDescription() {
  return TRANSLATE("Base 100%.");
}
std::string CharacterDerivedStatDefinitions::itemUsageValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.itemUsagePercent);
}

std::string CharacterDerivedStatDefinitions::foodConsumptionLabel() {
  return TRANSLATE("Food Consumption");
}
std::string CharacterDerivedStatDefinitions::foodConsumptionDescription() {
  return TRANSLATE("Base 100, reduced by Cooking training.");
}
std::string CharacterDerivedStatDefinitions::foodConsumptionValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.foodConsumption);
}

std::string CharacterDerivedStatDefinitions::firstAidLabel() { return TRANSLATE("First Aid"); }
std::string CharacterDerivedStatDefinitions::firstAidDescription() {
  return TRANSLATE("Base 1d4 per level.");
}
std::string CharacterDerivedStatDefinitions::firstAidValue(
    const CharacterDerivedStats& derived) {
  return "1d4 x " + std::to_string(derived.firstAidPerLevel);
}

std::string CharacterDerivedStatDefinitions::ingredientFindChanceLabel() {
  return TRANSLATE("Ingredient Find Chance");
}
std::string CharacterDerivedStatDefinitions::ingredientFindChanceDescription() {
  return TRANSLATE("Base 10%, plus Survival training.");
}
std::string CharacterDerivedStatDefinitions::ingredientFindChanceValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.ingredientFindChancePercent);
}

std::string CharacterDerivedStatDefinitions::foodConsumptionPerDayLabel() {
  return TRANSLATE("Food Consumption Per Day");
}
std::string CharacterDerivedStatDefinitions::foodConsumptionPerDayDescription() {
  return TRANSLATE("Base 100.");
}
std::string CharacterDerivedStatDefinitions::foodConsumptionPerDayValue(
    const CharacterDerivedStats& derived) {
  return std::to_string(derived.foodConsumptionPerDay);
}

} // namespace model
