#pragma once

#include "model/templates/AbilityTypes.h"
#include "lib/json.hpp"
#include <string>
#include <vector>

namespace db {

std::vector<model::Dice> parseDiceArray(const nlohmann::json& json, const std::string& fieldName);
model::TargetSelectInfo parseTargetSelectInfo(const nlohmann::json& json);
model::Stats parseStats(const nlohmann::json& json);
model::CurrentStats parseCurrentStats(const nlohmann::json& json);
model::Resistance parseResistance(const nlohmann::json& json);
model::AbilitySave parseAbilitySave(const nlohmann::json& json);
model::AbilityAttackDmg parseAbilityAttackDmg(const nlohmann::json& json);
model::AbilityAttack parseAbilityAttack(const nlohmann::json& json);
model::AbilityStatus parseAbilityStatus(const nlohmann::json& json);
model::AbilityRestore parseAbilityRestore(const nlohmann::json& json);
model::AbilityDepiction parseAbilityDepiction(const nlohmann::json& json);

} // namespace db
