#pragma once

#include "bmin/String.h"
#include "bmin/Map.h"
#include "model/templates/CharacterTemplate.h"

namespace db {

void loadCharacterTemplates(
    const bmin::String& charactersFilePath,
    bmin::Map<bmin::String, model::CharacterTemplate>& characterTemplates);

} // namespace db
