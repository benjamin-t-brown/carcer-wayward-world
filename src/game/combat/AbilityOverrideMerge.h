#pragma once

#include "bmin/DynArray.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"

namespace game {

// Whole-object replace of attacks[i].dmg from a real dmgOverrides[i].
// Index-replace only — do not port editor hole-fill / D6 / dmgStatMult=1 defaults.
// See ceditor/src/client/types/assets.ts (normalizeItemWeaponConfig,
// resolveItemWeaponDmgOverrides) and ceditor/src/client/types/ability.ts
// (mergeAbilityAttackDmg) for the index-replace rule only.
void mergeAbilityAttackDmgOverrides(
    model::AbilityTemplate& ability,
    const bmin::DynArray<model::AbilityAttackDmg>& dmgOverrides);

} // namespace game
