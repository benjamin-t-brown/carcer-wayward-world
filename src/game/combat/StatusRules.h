#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/stats/CharacterStats.h"
#include "model/templates/AbilityTypes.h"
#include "model/templates/StatusEffects.hpp"

namespace db {
class Database;
}

namespace game {

int calculateStatusDuration(const model::AbilityStatus& apply,
                            const model::StatusEffectTemplate& statusTemplate,
                            const model::CharacterStats& casterStats);

/** Adds or refreshes `apply.statusEffect` on `target`. Returns false if the
 * template is missing or duration is not positive. */
bool applyStatusEffect(model::CharacterInstance& target,
                       const model::AbilityStatus& apply,
                       const model::CharacterStats& casterStats,
                       const db::Database& database);

void collectStatusActionAbilities(const model::StatusEffectTemplate& statusTemplate,
                                  model::StatusEventType event,
                                  bmin::DynArray<bmin::String>& abilityNames);

/** Fires matching actions, then decrements remaining turns and drops expired. */
void collectTurnStartStatusAbilities(model::CharacterInstance& character,
                                     const db::Database& database,
                                     bmin::DynArray<bmin::String>& abilityNames);

} // namespace game
