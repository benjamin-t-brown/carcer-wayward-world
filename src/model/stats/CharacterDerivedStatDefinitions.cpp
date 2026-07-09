#include "model/stats/CharacterDerivedStatDefinitions.h"
#include "lib/sdl2w/L10n.h"
#include "lib/Types.h"

namespace model {

namespace {

String percentValue(int value) { return bmin::toString(value) + "%"; }

} // namespace

String CharacterDerivedStatDefinitions::derivedTitle() { return TRANSLATE("Derived"); }
String CharacterDerivedStatDefinitions::derivedSkillsTitle() {
  return TRANSLATE("Derived Skills");
}

String CharacterDerivedStatDefinitions::actionPointsLabel() {
  return TRANSLATE("Action Points");
}
String CharacterDerivedStatDefinitions::actionPointsDescription() {
  return TRANSLATE("Base 1.");
}
String CharacterDerivedStatDefinitions::actionPointsValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.actionPoints);
}

String CharacterDerivedStatDefinitions::mightDamageLabel() {
  return TRANSLATE("Might/Damage");
}
String CharacterDerivedStatDefinitions::mightDamageDescription() {
  return TRANSLATE("Base 1, plus Strength.");
}
String CharacterDerivedStatDefinitions::mightDamageValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.mightDamage);
}

String CharacterDerivedStatDefinitions::magicDamageLabel() {
  return TRANSLATE("Magic/Damage");
}
String CharacterDerivedStatDefinitions::magicDamageDescription() {
  return TRANSLATE("Base 0, plus Mind.");
}
String CharacterDerivedStatDefinitions::magicDamageValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.magicDamage);
}

String CharacterDerivedStatDefinitions::hpLabel() { return TRANSLATE("HP"); }
String CharacterDerivedStatDefinitions::hpDescription() {
  return TRANSLATE("Base 10, plus 5 per Constitution.");
}
String CharacterDerivedStatDefinitions::hpValue(const CharacterDerivedStats& derived) {
  return bmin::toString(derived.hp);
}

String CharacterDerivedStatDefinitions::attackHitChanceLabel() {
  return TRANSLATE("Attack Hit Chance");
}
String CharacterDerivedStatDefinitions::attackHitChanceDescription() {
  return TRANSLATE("Base 50%.");
}
String CharacterDerivedStatDefinitions::attackHitChanceValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.attackHitChancePercent);
}

String CharacterDerivedStatDefinitions::abilityPowerLabel() {
  return TRANSLATE("Ability Power");
}
String CharacterDerivedStatDefinitions::abilityPowerDescription() {
  return TRANSLATE("Base 0, from Magic Mastery training.");
}
String CharacterDerivedStatDefinitions::abilityPowerValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.abilityPower);
}

String CharacterDerivedStatDefinitions::damageReductionLabel() {
  return TRANSLATE("Damage Reduction");
}
String CharacterDerivedStatDefinitions::damageReductionDescription() {
  return TRANSLATE("Base 0, from Body Mastery training.");
}
String CharacterDerivedStatDefinitions::damageReductionValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.damageReduction);
}

String CharacterDerivedStatDefinitions::armorClassLabel() {
  return TRANSLATE("Armor Class");
}
String CharacterDerivedStatDefinitions::armorClassDescription() {
  return TRANSLATE("Base 10.");
}
String CharacterDerivedStatDefinitions::armorClassValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.armorClass);
}

String CharacterDerivedStatDefinitions::spellPotencyLabel() {
  return TRANSLATE("Spell Potency");
}
String CharacterDerivedStatDefinitions::spellPotencyDescription() {
  return TRANSLATE("Spell potency, status effect length, and field length. Base 1.");
}
String CharacterDerivedStatDefinitions::spellPotencyValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.spellPotency);
}

String CharacterDerivedStatDefinitions::manaLabel() { return TRANSLATE("Mana"); }
String CharacterDerivedStatDefinitions::manaDescription() {
  return TRANSLATE("Base 10, plus Magic Mastery and Focus.");
}
String CharacterDerivedStatDefinitions::manaValue(const CharacterDerivedStats& derived) {
  return bmin::toString(derived.maxMana);
}

String CharacterDerivedStatDefinitions::resistancesLabel() {
  return TRANSLATE("Resistances");
}
String CharacterDerivedStatDefinitions::resistancesDescription() {
  return TRANSLATE("Base none.");
}
String CharacterDerivedStatDefinitions::resistancesValue(
    const CharacterDerivedStats&) {
  return TRANSLATE("None");
}

String CharacterDerivedStatDefinitions::jumpDistanceLabel() {
  return TRANSLATE("Jump Distance");
}
String CharacterDerivedStatDefinitions::jumpDistanceDescription() {
  return TRANSLATE("Base 2.");
}
String CharacterDerivedStatDefinitions::jumpDistanceValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.jumpDistance);
}

String CharacterDerivedStatDefinitions::healingEffectivenessLabel() {
  return TRANSLATE("Healing Effectiveness");
}
String CharacterDerivedStatDefinitions::healingEffectivenessDescription() {
  return TRANSLATE("Base 100%.");
}
String CharacterDerivedStatDefinitions::healingEffectivenessValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.healingEffectivenessPercent);
}

String CharacterDerivedStatDefinitions::statusEffectShieldLabel() {
  return TRANSLATE("Status Effect Shield");
}
String CharacterDerivedStatDefinitions::statusEffectShieldDescription() {
  return TRANSLATE("Base 0.");
}
String CharacterDerivedStatDefinitions::statusEffectShieldValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.statusEffectShield);
}

String CharacterDerivedStatDefinitions::materiaSlotsLabel() {
  return TRANSLATE("Materia Slots");
}
String CharacterDerivedStatDefinitions::materiaSlotsDescription() {
  return TRANSLATE("Base 0.");
}
String CharacterDerivedStatDefinitions::materiaSlotsValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.materiaSlots);
}

String CharacterDerivedStatDefinitions::shieldBonusLabel() {
  return TRANSLATE("Shield Bonus");
}
String CharacterDerivedStatDefinitions::shieldBonusDescription() {
  return TRANSLATE("Base 0.");
}
String CharacterDerivedStatDefinitions::shieldBonusValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.shieldBonus);
}

String CharacterDerivedStatDefinitions::enemyVisionRangeLabel() {
  return TRANSLATE("Enemy Vision Range");
}
String CharacterDerivedStatDefinitions::enemyVisionRangeDescription() {
  return TRANSLATE("Affected by Stealth training.");
}
String CharacterDerivedStatDefinitions::enemyVisionRangeValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.enemyVisionRange);
}

String CharacterDerivedStatDefinitions::mageLoreLabel() {
  return TRANSLATE("Mage Lore");
}
String CharacterDerivedStatDefinitions::mageLoreDescription() {
  return TRANSLATE("Affected by Magic Item Use training.");
}
String CharacterDerivedStatDefinitions::mageLoreValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.mageLore);
}

String CharacterDerivedStatDefinitions::toolUseLabel() { return TRANSLATE("Tool Use"); }
String CharacterDerivedStatDefinitions::toolUseDescription() {
  return TRANSLATE("Affected by Trickery training.");
}
String CharacterDerivedStatDefinitions::toolUseValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.toolUse);
}

String CharacterDerivedStatDefinitions::tradeDiscountLabel() {
  return TRANSLATE("Trade Discount");
}
String CharacterDerivedStatDefinitions::tradeDiscountDescription() {
  return TRANSLATE("Base 0%, plus 1% per Social level.");
}
String CharacterDerivedStatDefinitions::tradeDiscountValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.tradeDiscountPercent);
}

String CharacterDerivedStatDefinitions::itemUsageLabel() {
  return TRANSLATE("Item Usage");
}
String CharacterDerivedStatDefinitions::itemUsageDescription() {
  return TRANSLATE("Base 100%.");
}
String CharacterDerivedStatDefinitions::itemUsageValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.itemUsagePercent);
}

String CharacterDerivedStatDefinitions::foodConsumptionLabel() {
  return TRANSLATE("Food Consumption");
}
String CharacterDerivedStatDefinitions::foodConsumptionDescription() {
  return TRANSLATE("Base 100, reduced by Cooking training.");
}
String CharacterDerivedStatDefinitions::foodConsumptionValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.foodConsumption);
}

String CharacterDerivedStatDefinitions::firstAidLabel() { return TRANSLATE("First Aid"); }
String CharacterDerivedStatDefinitions::firstAidDescription() {
  return TRANSLATE("Base 1d4 per level.");
}
String CharacterDerivedStatDefinitions::firstAidValue(
    const CharacterDerivedStats& derived) {
  return "1d4 x " + bmin::toString(derived.firstAidPerLevel);
}

String CharacterDerivedStatDefinitions::ingredientFindChanceLabel() {
  return TRANSLATE("Ingredient Find Chance");
}
String CharacterDerivedStatDefinitions::ingredientFindChanceDescription() {
  return TRANSLATE("Base 10%, plus Survival training.");
}
String CharacterDerivedStatDefinitions::ingredientFindChanceValue(
    const CharacterDerivedStats& derived) {
  return percentValue(derived.ingredientFindChancePercent);
}

String CharacterDerivedStatDefinitions::foodConsumptionPerDayLabel() {
  return TRANSLATE("Food Consumption Per Day");
}
String CharacterDerivedStatDefinitions::foodConsumptionPerDayDescription() {
  return TRANSLATE("Base 100.");
}
String CharacterDerivedStatDefinitions::foodConsumptionPerDayValue(
    const CharacterDerivedStats& derived) {
  return bmin::toString(derived.foodConsumptionPerDay);
}

} // namespace model
