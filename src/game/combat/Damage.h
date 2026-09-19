#pragma once

#include "model/instances/CharacterInstance.hpp"
#include "model/stats/CharacterStats.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"

namespace game {

struct CalculatedAbilityDamageResult {
  bool didHit = true;
  int damage = 0;
  int damageReduced = 0;
};

CalculatedAbilityDamageResult calculateAttackDamage(
    const model::AbilityAttack& attack,
    const model::CharacterStats& attackerStats);

CalculatedAbilityDamageResult calculateAbilityDamage(
    const model::AbilityDamage& abilityDamage,
    const model::CharacterStats& attackerStats);

CalculatedAbilityDamageResult calculateAbilityDamage(
    const model::AbilityDamage& abilityDamage,
    const model::CharacterInstance& attacker,
    const model::CharacterInstance& victim);

int calculateAbilityTemplateDamage(const model::AbilityTemplate& ability,
                                   const model::CharacterStats& attackerStats);

int calculateAbilityRestoreAmount(const model::AbilityRestore& restore,
                                  const model::CharacterStats& casterStats);

int calculateAbilityTemplateHpDelta(const model::AbilityTemplate& ability,
                                    const model::CharacterStats& casterStats);

} // namespace game
