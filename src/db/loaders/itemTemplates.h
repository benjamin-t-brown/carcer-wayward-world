#pragma once

#include "types/Items.h"
#include <string>
#include <unordered_map>

namespace db {

void loadItemTemplates(
    const std::string& itemsFilePath,
    std::unordered_map<std::string, types::ItemTemplate>& itemTemplates);

} // namespace db