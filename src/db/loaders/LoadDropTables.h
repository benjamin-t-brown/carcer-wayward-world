#pragma once

#include "bmin/String.h"
#include "bmin/Map.h"
#include "model/templates/DropTables.hpp"

namespace db {

void loadDropTables(const bmin::String& dropTablesFilePath,
                    bmin::Map<bmin::String, model::DropTableTemplate>& dropTables);

} // namespace db
