#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"
#include "model/templates/CharacterTemplate.h"

namespace db {

void loadCharacterTemplates(
    const String& charactersFilePath,
    bmin::Map<String, model::CharacterTemplate>& characterTemplates);

} // namespace db
