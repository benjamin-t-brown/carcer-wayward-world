#pragma once

#include "model/templates/CombatTypes.h"
#include <optional>
#include <string>
#include <vector>

namespace model {

struct StatusEffectEvent {
  StatusEventType type = StatusEventType::STATUS_EVENT_ON_APPLIED;
  StatusEffectCondition condition = StatusEffectCondition::CONDITION_ALWAYS;
};

struct StatusEffectAction {
  StatusActionTargetType statusActionTargetType =
      StatusActionTargetType::STATUS_ACTION_TARGET_SELF;
  std::string abilityName;
};

struct StatusEffectTemplate {
  std::string name;
  std::string description;
  int duration = 0;
  std::vector<StatusEffectEvent> events;
  std::optional<Stats> applyBonuses;
  std::optional<CurrentStats> applyCurrentStatChange;
  std::vector<Resistance> applyResistances;
  std::optional<TargetSelectInfo> targetInfo;
  std::vector<StatusEffectAction> actions;
};

} // namespace model
