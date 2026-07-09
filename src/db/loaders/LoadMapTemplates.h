#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"
#include "model/templates/Maps.h"

namespace db {

void loadMapTemplates(const String& mapsFilePath,
                      bmin::Map<String, model::CarcerMapTemplate>& mapTemplates);

} // namespace db
