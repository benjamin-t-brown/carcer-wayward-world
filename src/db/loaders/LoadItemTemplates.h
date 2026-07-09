#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"
#include "model/templates/Items.h"

namespace db {

void loadItemTemplates(const String& itemsFilePath,
                       bmin::Map<String, model::ItemTemplate>& itemTemplates);

} // namespace db
