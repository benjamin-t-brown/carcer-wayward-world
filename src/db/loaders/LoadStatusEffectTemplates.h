#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"
#include "model/templates/StatusEffects.h"

namespace db {

void loadStatusEffectTemplates(
    const String& statusEffectsFilePath,
    bmin::Map<String, model::StatusEffectTemplate>& statusEffectTemplates);

} // namespace db
