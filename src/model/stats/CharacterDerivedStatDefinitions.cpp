#include "model/stats/CharacterDerivedStatDefinitions.h"
#include "sdl2w/L10n.h"
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
