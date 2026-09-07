#include <functional>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string_view>
import carcer.db;
import sdl2w;
import bmin.string_interop;
#include "macros.h"

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

    const auto singeIt = abilityTemplates.find(bmin::String("SPELL_SINGE"));
    if (singeIt == abilityTemplates.end()) {
      LOG(ERROR) << "Missing SPELL_SINGE ability" << LOG_ENDL;
      return 1;
    }
    if (singeIt->value.damages.empty()) {
      LOG(ERROR) << "SPELL_SINGE should have damages" << LOG_ENDL;
      return 1;
    }
    const auto& singeDamage = singeIt->value.damages[0];
    if (singeDamage.damageType != model::DamageType::DAMAGE_TYPE_HEAT) {
      LOG(ERROR) << "SPELL_SINGE damageType should be DAMAGE_TYPE_HEAT" << LOG_ENDL;
      return 1;
    }
    if (singeDamage.dmgDice.size() != 1 || singeDamage.dmgDice[0] != model::Dice::D4) {
      LOG(ERROR) << "SPELL_SINGE dmgDice should be one D4" << LOG_ENDL;
      return 1;
    }
    if (singeDamage.dmgBonus != 2) {
      LOG(ERROR) << "SPELL_SINGE dmgBonus should be 2" << LOG_ENDL;
      return 1;
    }
    if (singeDamage.dmgStat != model::StatsEnum::STAT_MND) {
      LOG(ERROR) << "SPELL_SINGE dmgStat should be STAT_MND" << LOG_ENDL;
      return 1;
    }
    if (singeDamage.dmgStatMult != 0.25f) {
      LOG(ERROR) << "SPELL_SINGE dmgStatMult should be 0.25" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadAbilityTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
