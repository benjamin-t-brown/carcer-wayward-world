#pragma once

#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/Spells.h"

namespace db {

void loadSpellTemplates(const bmin::String& spellsFilePath,
                        bmin::Map<bmin::String, model::SpellTemplate>& spellTemplates);

} // namespace db
