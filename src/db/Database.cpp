#include "Database.h"
#include "lib/sdl2w/Logger.h"
#include "loaders/LoadItemTemplates.h"
#include <stdexcept>
#include <string>

namespace db {

Database::Database() {}

void Database::load() {
  LOG(INFO) << "Loading database..." << LOG_ENDL;
  loadItemTemplates("assets/db/items.txt", itemTemplates);
  LOG(INFO) << "Loaded database." << LOG_ENDL;
}

const model::ItemTemplate& Database::getItemTemplate(const std::string& itemName) const {
  if (itemTemplates.find(itemName) == itemTemplates.end()) {
    throw std::runtime_error("Item template not found: " + itemName);
  }
  return itemTemplates.at(itemName);
}

void Database::addItemTemplate(const model::ItemTemplate& itemTemplate) {
  itemTemplates[itemTemplate.name] = itemTemplate;
}

} // namespace db