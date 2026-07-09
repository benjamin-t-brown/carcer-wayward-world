#pragma once

#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/Items.h"

namespace db {

void loadItemTemplates(const bmin::String& itemsFilePath,
                       bmin::Map<bmin::String, model::ItemTemplate>& itemTemplates);

} // namespace db
