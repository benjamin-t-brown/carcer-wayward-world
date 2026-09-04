module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.model.templates:Abilities;
export import bmin.containers;
import bmin.string_interop;
export import :AbilityTypes;

export {

// --- from model/templates/Abilities.h ---
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
  bmin::DynArray<AbilityDamage> damages;
};

} // namespace model

} // export
