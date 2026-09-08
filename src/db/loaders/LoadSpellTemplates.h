#pragma once

#include "bmin/String.h"
#include "bmin/Map.h"
#include "model/templates/Spells.hpp"

namespace db {

void loadSpellTemplates(const bmin::String& spellsFilePath,
                        bmin::Map<bmin::String, model::SpellTemplate>& spellTemplates);

} // namespace db
