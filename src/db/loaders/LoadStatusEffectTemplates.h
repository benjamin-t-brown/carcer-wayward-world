#pragma once

#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/StatusEffects.h"

namespace db {

void loadStatusEffectTemplates(
    const bmin::String& statusEffectsFilePath,
    bmin::Map<bmin::String, model::StatusEffectTemplate>& statusEffectTemplates);

} // namespace db
