#pragma once

#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/Maps.h"

namespace db {

void loadMapTemplates(const bmin::String& mapsFilePath,
                      bmin::Map<bmin::String, model::CarcerMapTemplate>& mapTemplates);

} // namespace db
