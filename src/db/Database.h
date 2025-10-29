#pragma once

#include "types/Items.h"
#include <unordered_map>

namespace db {

class Database {
private:
  std::unordered_map<std::string, types::ItemTemplate> itemTemplates;

public:
  Database();
  ~Database() = default;

  const types::ItemTemplate& getItemTemplate(const std::string& itemId) const;
  void load();
};

} // namespace db