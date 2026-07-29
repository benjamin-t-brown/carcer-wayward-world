#pragma once

#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/MapGrids.h"

namespace db {

void loadMapGridTemplates(const bmin::String& mapGridsFilePath,
                          bmin::Map<bmin::String, model::MapGridTemplate>& mapGridTemplates);

} // namespace db
