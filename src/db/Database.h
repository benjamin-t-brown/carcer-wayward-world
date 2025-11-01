#pragma once

#include "model/Items.h"
#include "model/SpecialEvents.h"
#include <unordered_map>

namespace db {

class Database {
private:
  std::unordered_map<std::string, model::ItemTemplate> itemTemplates;
  std::unordered_map<std::string, model::GameEvent> gameEvents;

public:
  Database();
  ~Database() = default;

  const model::ItemTemplate& getItemTemplate(const std::string& itemName) const;
  void addItemTemplate(const model::ItemTemplate& itemTemplate);
  const model::GameEvent& getGameEvent(const std::string& eventId) const;
  void addGameEvent(const model::GameEvent& gameEvent);
  void load();
};

} // namespace db