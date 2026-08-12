#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "model/templates/RuneTypes.h"

namespace model {

struct SpellRuneRequirement {
  RuneType type = RuneType::HEAT;
  int count = 1;
};

struct SpellTemplate {
  bmin::String name;
  bmin::String label;
  bmin::String description;
  bmin::String icon;
  bmin::String abilityName;
  bmin::DynArray<SpellRuneRequirement> requiredRunes;
};

} // namespace model
