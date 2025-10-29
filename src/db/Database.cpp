#include "Database.h"
#include "lib/sdl2w/Logger.h"
#include "loaders/itemTemplates.h"
#include <stdexcept>
#include <string>

namespace db {

Database::Database() {}

void Database::load() {
  LOG(INFO) << "Loading database..." << LOG_ENDL;
  loadItemTemplates("assets/db/items.txt", itemTemplates);
  LOG(INFO) << "Loaded database." << LOG_ENDL;
}

const types::ItemTemplate& Database::getItemTemplate(const std::string& itemId) const {
  if (itemTemplates.find(itemId) == itemTemplates.end()) {
    throw std::runtime_error("Item template not found: " + itemId);
  }
  return itemTemplates.at(itemId);
}

} // namespace db