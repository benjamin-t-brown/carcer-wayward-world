#pragma once

#include "model/templates/AbilityTypes.h"
#include <string>
#include <vector>

namespace model {

struct AbilityTemplate {
  std::string name;
  std::string label;
  std::string description;
  std::string icon;
  AbilityType type = AbilityType::ABILITY_ATTACK;
  TargetSelectInfo targetSelect;
  int apCost = 0;
  AbilityCostType costType = AbilityCostType::ABILITY_COST_NONE;
  int costValue = 0;
  AbilityDepiction depiction;
  std::vector<AbilityAttack> attacks;
  std::vector<AbilityStatus> statuses;
  std::vector<AbilityRestore> restores;
};

} // namespace model
