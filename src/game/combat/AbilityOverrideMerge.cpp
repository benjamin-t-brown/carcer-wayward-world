#include "game/combat/AbilityOverrideMerge.h"

namespace game {

namespace {

bool isLegacyDmgOverridePad(const model::AbilityAttackDmg& dmg) {
  return dmg.dmgDice.empty() && dmg.dmgBonus == 0 && dmg.attackBonus == 0 &&
         dmg.dmgStatMult == 0.f;
}

} // namespace

void mergeAbilityAttackDmgOverrides(
    model::AbilityTemplate& ability,
    const bmin::DynArray<model::AbilityAttackDmg>& dmgOverrides) {
  const auto count = ability.attacks.size() < dmgOverrides.size()
                         ? ability.attacks.size()
                         : dmgOverrides.size();
  for (size_t i = 0; i < count; ++i) {
    const auto& dmgOverride = dmgOverrides[i];
    if (isLegacyDmgOverridePad(dmgOverride)) {
      continue;
    }
    ability.attacks[i].dmg = dmgOverride;
  }
}

void mergeAbilityRestoreOverrides(
    model::AbilityTemplate& ability,
    const bmin::DynArray<model::AbilityRestore>& restoreOverrides) {
  if (ability.restores.empty()) {
    if (!restoreOverrides.empty()) {
      ability.restores = restoreOverrides;
    }
    return;
  }
  const auto count = ability.restores.size() < restoreOverrides.size()
                         ? ability.restores.size()
                         : restoreOverrides.size();
  for (size_t i = 0; i < count; ++i) {
    ability.restores[i] = restoreOverrides[i];
  }
}

} // namespace game

