module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>

export module carcer.model.templates.StatusEffects;
export import bmin.containers;
import bmin.string_interop;
export import carcer.model.templates.AbilityTypes;

export {

// --- from model/templates/StatusEffects.h ---
namespace model {

struct StatusEffectEvent {
  StatusEventType type = StatusEventType::STATUS_EVENT_ON_APPLIED;
  StatusEffectCondition condition = StatusEffectCondition::CONDITION_ALWAYS;
};

struct StatusEffectDurationScale {
  StatsEnum durationStat = StatsEnum::STAT_MND;
  int durationStatMult = 0;
};

struct StatusEffectAction {
  StatusActionTargetType statusActionTargetType =
      StatusActionTargetType::STATUS_ACTION_TARGET_SELF;
  bmin::String abilityName;
  bmin::DynArray<StatusEffectEvent> events;
};

struct StatusEffectTemplate {
  bmin::String name;
  bmin::String description;
  // Default turns before apply-time modifiers (applier spellPotency, victim shield, feats).
  int baseDuration = 0;
  std::optional<StatusEffectDurationScale> durationScale;
  std::optional<Stats> applyBonuses;
  std::optional<CurrentStats> applyCurrentStatChange;
  bmin::DynArray<Resistance> applyResistances;
  bmin::DynArray<StatusEffectAction> actions;
};

} // namespace model

} // export
