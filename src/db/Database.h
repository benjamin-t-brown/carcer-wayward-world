#pragma once

#include "model/Items.h"
#include <unordered_map>

namespace db {

class Database {
private:
  std::unordered_map<std::string, model::ItemTemplate> itemTemplates;

public:
  Database();
  ~Database() = default;

  const model::ItemTemplate& getItemTemplate(const std::string& itemId) const;
  void load();
};

} // namespace db