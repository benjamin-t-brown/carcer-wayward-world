#pragma once

#include <string>

namespace db {
std::string getItemIconSpriteName(const std::string& itemIcon,
                                  const std::string& spriteSheet = "ui_item_icons");
}; // namespace db