#pragma once

#include "model/templates/StatusEffects.h"
#include <string>
#include <unordered_map>

namespace db {

void loadStatusEffectTemplates(
    const std::string& statusEffectsFilePath,
    std::unordered_map<std::string, model::StatusEffectTemplate>& statusEffectTemplates);

} // namespace db
