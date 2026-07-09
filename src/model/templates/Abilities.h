#pragma once

#include "model/templates/AbilityTypes.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

struct AbilityTemplate {
  bmin::String name;
  bmin::String label;
  bmin::String description;
  bmin::String icon;
  AbilityType type = AbilityType::ABILITY_ATTACK;
  TargetSelectInfo targetSelect;
  int apCost = 0;
  AbilityCostType costType = AbilityCostType::ABILITY_COST_NONE;
  int costValue = 0;
  AbilityDepiction depiction;
  bmin::DynArray<AbilityAttack> attacks;
  bmin::DynArray<AbilityStatus> statuses;
  bmin::DynArray<AbilityRestore> restores;
};

} // namespace model
