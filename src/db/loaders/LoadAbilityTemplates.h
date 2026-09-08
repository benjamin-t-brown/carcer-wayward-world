#pragma once

#include "bmin/String.h"
#include "bmin/Map.h"
#include "model/templates/Abilities.hpp"

namespace db {

void loadAbilityTemplates(const bmin::String& abilitiesFilePath,
                          bmin::Map<bmin::String, model::AbilityTemplate>& abilityTemplates);

} // namespace db
