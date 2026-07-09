#pragma once

#include "bmin/String.h"
#include "lib/Json.h"
#include "model/templates/AbilityTypes.h"

namespace db {

bmin::DynArray<model::Dice> parseDiceArray(const Json& json, const bmin::String& fieldName);
model::TargetSelectInfo parseTargetSelectInfo(const Json& json);
model::Stats parseStats(const Json& json);
model::CurrentStats parseCurrentStats(const Json& json);
model::Resistance parseResistance(const Json& json);
model::AbilitySave parseAbilitySave(const Json& json);
model::AbilityAttackDmg parseAbilityAttackDmg(const Json& json);
model::AbilityAttack parseAbilityAttack(const Json& json);
model::AbilityStatus parseAbilityStatus(const Json& json);
model::AbilityRestore parseAbilityRestore(const Json& json);
model::AbilityDepiction parseAbilityDepiction(const Json& json);

} // namespace db
