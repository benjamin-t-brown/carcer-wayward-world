#pragma once

#include "model/templates/CharacterTemplate.h"
#include <string>
#include <unordered_map>

namespace db {

void loadCharacterTemplates(
    const std::string& charactersFilePath,
    std::unordered_map<std::string, model::CharacterTemplate>& characterTemplates);

} // namespace db
