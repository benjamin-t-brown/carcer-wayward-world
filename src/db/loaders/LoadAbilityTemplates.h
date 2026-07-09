#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"
#include "model/templates/Abilities.h"

namespace db {

void loadAbilityTemplates(const String& abilitiesFilePath,
                          bmin::Map<String, model::AbilityTemplate>& abilityTemplates);

} // namespace db
