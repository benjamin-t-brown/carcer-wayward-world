#pragma once

#include <string>

namespace model {

enum class StatusEffectType { MODIFY_RESISTANCE_FIRE_PLUS_1 };

struct StatusEffectTemplate {
  StatusEffectType statusEffectType;
  std::string name;
  std::string label;
  std::string description;
};

} // namespace model