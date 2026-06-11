#pragma once

#include "model/templates/Maps.h"
#include <string>
#include <unordered_map>

namespace db {

void loadMapTemplates(const std::string& mapsFilePath,
                      std::unordered_map<std::string, model::CarcerMapTemplate>& mapTemplates);

} // namespace db
