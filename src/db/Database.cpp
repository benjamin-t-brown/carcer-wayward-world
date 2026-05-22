#include "Database.h"
#include "lib/sdl2w/Logger.h"
#include "loaders/LoadItemTemplates.h"
#include <stdexcept>
#include <string>

namespace db {

Database::Database() {}

void Database::load() {
  LOG(INFO) << "Loading database..." << LOG_ENDL;
  loadItemTemplates("assets/db/items.json", itemTemplates);
  // load special events
  LOG(INFO) << "Loaded database." << LOG_ENDL;
}

const model::ItemTemplate& Database::getItemTemplate(std::string_view itemName) const {
  if (itemTemplates.find(std::string(itemName)) == itemTemplates.end()) {
    throw std::runtime_error("Item template not found: " + std::string(itemName));
  }
  return itemTemplates.at(std::string(itemName));
}

void Database::addItemTemplate(const model::ItemTemplate& itemTemplate) {
  itemTemplates[itemTemplate.name] = itemTemplate;
}

const model::GameEvent& Database::getGameEvent(std::string_view eventId) const {
  if (gameEvents.find(std::string(eventId)) == gameEvents.end()) {
    throw std::runtime_error("Game event not found: " + std::string(eventId));
  }
  return gameEvents.at(std::string(eventId));
}

void Database::addGameEvent(const model::GameEvent& gameEvent) {
  gameEvents[gameEvent.id] = gameEvent;
}
} // namespace db