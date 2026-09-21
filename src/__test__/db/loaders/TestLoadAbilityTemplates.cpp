#include "db/loaders/LoadAbilityTemplates.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"
#include "bmin/Map.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadAbilityTemplates" << LOG_ENDL;

  bmin::Map<bmin::String, model::AbilityTemplate> abilityTemplates;

  try {
    db::loadAbilityTemplates("assets/db/abilities.json", abilityTemplates);
    LOG(INFO) << "Loaded " << abilityTemplates.size() << " ability templates" << LOG_ENDL;

    const auto meleeIt = abilityTemplates.find(bmin::String("MELEE_ATTACK_METAL_SWORD"));
    if (meleeIt == abilityTemplates.end()) {
      LOG(ERROR) << "Missing MELEE_ATTACK_METAL_SWORD ability" << LOG_ENDL;
      return 1;
    }
    if (meleeIt->value.attacks.empty()) {
      LOG(ERROR) << "MELEE_ATTACK_METAL_SWORD should have attacks" << LOG_ENDL;
      return 1;
    }
    if (meleeIt->value.attacks[0].damageType != model::DamageType::DAMAGE_TYPE_EDGED) {
      LOG(ERROR) << "MELEE_ATTACK_METAL_SWORD attack damageType should be DAMAGE_TYPE_EDGED"
                 << LOG_ENDL;
      return 1;
    }
    if (!meleeIt->value.damages.empty()) {
      LOG(ERROR) << "MELEE_ATTACK_METAL_SWORD should have empty damages" << LOG_ENDL;
      return 1;
    }

    const auto defaultIt = abilityTemplates.find(bmin::String("MELEE_ATTACK_DEFAULT"));
    if (defaultIt == abilityTemplates.end()) {
      LOG(ERROR) << "Missing MELEE_ATTACK_DEFAULT ability" << LOG_ENDL;
      return 1;
    }
    if (defaultIt->value.attacks.empty()) {
      LOG(ERROR) << "MELEE_ATTACK_DEFAULT should have attacks" << LOG_ENDL;
      return 1;
    }
    if (defaultIt->value.depiction.dmgAnim.empty() ||
        defaultIt->value.depiction.dmgSound.empty()) {
      LOG(ERROR) << "MELEE_ATTACK_DEFAULT should have depiction anim and sound"
                 << LOG_ENDL;
      return 1;
    }

    const auto singeIt = abilityTemplates.find(bmin::String("SPELL_SINGE"));
    if (singeIt == abilityTemplates.end()) {
      LOG(ERROR) << "Missing SPELL_SINGE ability" << LOG_ENDL;
      return 1;
    }
    if (!singeIt->value.damages.empty()) {
      LOG(ERROR) << "SPELL_SINGE should have empty damages" << LOG_ENDL;
      return 1;
    }
    if (singeIt->value.depiction.dmgTextColor != bmin::String("#111111")) {
      LOG(ERROR) << "SPELL_SINGE dmgTextColor should be #111111" << LOG_ENDL;
      return 1;
    }
    if (singeIt->value.statuses.empty() ||
        singeIt->value.statuses[0].statusEffect != bmin::String("BURNING")) {
      LOG(ERROR) << "SPELL_SINGE should apply BURNING" << LOG_ENDL;
      return 1;
    }

    const auto burnIt = abilityTemplates.find(bmin::String("SE_BURNING_1"));
    if (burnIt == abilityTemplates.end()) {
      LOG(ERROR) << "Missing SE_BURNING_1 ability" << LOG_ENDL;
      return 1;
    }
    if (burnIt->value.damages.empty()) {
      LOG(ERROR) << "SE_BURNING_1 should have damages" << LOG_ENDL;
      return 1;
    }
    const auto& burnDamage = burnIt->value.damages[0];
    if (burnDamage.damageType != model::DamageType::DAMAGE_TYPE_HEAT) {
      LOG(ERROR) << "SE_BURNING_1 damageType should be DAMAGE_TYPE_HEAT" << LOG_ENDL;
      return 1;
    }
    if (burnDamage.dmgDice.size() != 1 || burnDamage.dmgDice[0] != model::Dice::D6) {
      LOG(ERROR) << "SE_BURNING_1 dmgDice should be one D6" << LOG_ENDL;
      return 1;
    }
    if (burnDamage.dmgBonus != 1) {
      LOG(ERROR) << "SE_BURNING_1 dmgBonus should be 1" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadAbilityTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
