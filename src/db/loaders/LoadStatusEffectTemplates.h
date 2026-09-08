#pragma once

#include "bmin/String.h"
#include "bmin/Map.h"
#include "model/templates/StatusEffects.hpp"

namespace db {

void loadStatusEffectTemplates(
    const bmin::String& statusEffectsFilePath,
    bmin::Map<bmin::String, model::StatusEffectTemplate>& statusEffectTemplates);

} // namespace db
