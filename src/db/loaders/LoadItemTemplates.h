#pragma once

#include "model/Items.h"
#include <string>
#include <unordered_map>

namespace db {

void loadItemTemplates(
    const std::string& itemsFilePath,
    std::unordered_map<std::string, model::ItemTemplate>& itemTemplates);

} // namespace db

