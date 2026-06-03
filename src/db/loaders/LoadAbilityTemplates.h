#pragma once

#include "model/templates/Abilities.h"
#include <string>
#include <unordered_map>

namespace db {

void loadAbilityTemplates(const std::string& abilitiesFilePath,
                          std::unordered_map<std::string, model::AbilityTemplate>& abilityTemplates);

} // namespace db
