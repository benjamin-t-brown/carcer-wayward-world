#pragma once

#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/Abilities.h"

namespace db {

void loadAbilityTemplates(const bmin::String& abilitiesFilePath,
                          bmin::Map<bmin::String, model::AbilityTemplate>& abilityTemplates);

} // namespace db
