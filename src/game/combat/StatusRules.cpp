#include "game/combat/StatusRules.h"
#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "sdl2w/Logger.h"

namespace game {

namespace {

int statValue(const model::CharacterStats& stats, model::StatsEnum which) {
  switch (which) {
  case model::StatsEnum::STAT_STR:
    return stats.generic.str;
  case model::StatsEnum::STAT_MND:
    return stats.generic.mnd;
  case model::StatsEnum::STAT_CON:
    return stats.generic.con;
  case model::StatsEnum::STAT_AGI:
    return stats.generic.agi;
  case model::StatsEnum::STAT_LCK:
    return stats.generic.lck;
  }
  return 0;
}

model::AppliedStatusEffect* findAppliedStatus(model::CharacterInstance& character,
                                              const bmin::String& statusName) {
  for (size_t i = 0; i < character.statusEffects.size(); i++) {
    if (character.statusEffects[i].statusEffectName == statusName) {
      return &character.statusEffects[i];
    }
  }
  return nullptr;
}

} // namespace

int calculateStatusDuration(const model::AbilityStatus& apply,
                            const model::StatusEffectTemplate& statusTemplate,
                            const model::CharacterStats& casterStats) {
  auto duration = apply.baseDuration.has_value() ? *apply.baseDuration
                                                 : statusTemplate.baseDuration;
  if (apply.durationBonus.has_value()) {
    duration += *apply.durationBonus;
  }
  if (statusTemplate.durationScale.has_value()) {
    const auto& scale = *statusTemplate.durationScale;
    const auto scaled =
        static_cast<float>(statValue(casterStats, scale.durationStat)) *
        scale.durationStatMult;
    duration += static_cast<int>(scaled);
  }
  return duration;
}

bool applyStatusEffect(model::CharacterInstance& target,
                       const model::AbilityStatus& apply,
                       const model::CharacterStats& casterStats,
                       const db::Database& database) {
  if (apply.statusEffect.empty()) {
    return false;
  }
  const auto* statusTemplate =
      database.findStatusEffectTemplate(bmin::toStringView(apply.statusEffect));
  if (statusTemplate == nullptr) {
    LOG(ERROR) << "applyStatusEffect: missing template " << apply.statusEffect
               << LOG_ENDL;
    return false;
  }
  const auto duration = calculateStatusDuration(apply, *statusTemplate, casterStats);
  if (duration <= 0) {
    return false;
  }

  if (auto* existing = findAppliedStatus(target, apply.statusEffect)) {
    existing->remainingTurns = duration;
    LOG(DEBUG) << "applyStatusEffect: refresh " << apply.statusEffect << " on "
               << target.id << " remainingTurns=" << duration << LOG_ENDL;
    return true;
  }

  model::AppliedStatusEffect applied;
  applied.statusEffectName = apply.statusEffect;
  applied.remainingTurns = duration;
  target.statusEffects.pushBack(std::move(applied));
  LOG(DEBUG) << "applyStatusEffect: apply " << apply.statusEffect << " on " << target.id
             << " remainingTurns=" << duration << LOG_ENDL;
  return true;
}

void collectStatusActionAbilities(const model::StatusEffectTemplate& statusTemplate,
                                  model::StatusEventType event,
                                  bmin::DynArray<bmin::String>& abilityNames) {
  for (const auto& action : statusTemplate.actions) {
    if (action.abilityName.empty()) {
      continue;
    }
    auto matches = false;
    for (const auto& actionEvent : action.events) {
      if (actionEvent.type == event &&
          actionEvent.condition == model::StatusEffectCondition::CONDITION_ALWAYS) {
        matches = true;
        break;
      }
    }
    if (matches) {
      abilityNames.pushBack(action.abilityName);
    }
  }
}

void collectTurnStartStatusAbilities(model::CharacterInstance& character,
                                     const db::Database& database,
                                     bmin::DynArray<bmin::String>& abilityNames) {
  for (size_t i = 0; i < character.statusEffects.size();) {
    auto& applied = character.statusEffects[i];
    const auto* statusTemplate =
        database.findStatusEffectTemplate(bmin::toStringView(applied.statusEffectName));
    if (statusTemplate != nullptr) {
      collectStatusActionAbilities(
          *statusTemplate, model::StatusEventType::STATUS_EVENT_ON_TURN_START, abilityNames);
    }
    applied.remainingTurns -= 1;
    if (applied.remainingTurns <= 0) {
      character.statusEffects.erase(i);
      continue;
    }
    i++;
  }
}

} // namespace game
