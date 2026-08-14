#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/templates/AbilityTypes.h"

namespace game {

struct CalculatedAbilityDamageResult {
  bool didHit = true;
  int damage = 0;
  int damageReduced = 0;
};

CalculatedAbilityDamageResult calculateAttackDamage(
    const model::AbilityAttack& attack,
    const model::CharacterInstance& attacker,
    const model::CharacterInstance& target);

CalculatedAbilityDamageResult calculateAbilityDamage(
    const model::AbilityDamage& abilityDamage,
    const model::CharacterInstance& attacker,
    const model::CharacterInstance& victim);

} // namespace game