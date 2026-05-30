#pragma once

#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/SpecialEvents.h"
#include <unordered_map>

namespace db {

class Database {
private:
  std::unordered_map<std::string, model::ItemTemplate> itemTemplates;
  std::unordered_map<std::string, model::CharacterTemplate> characterTemplates;
  std::unordered_map<std::string, model::GameEvent> gameEvents;

public:
  Database();
  ~Database() = default;

  const model::ItemTemplate& getItemTemplate(std::string_view itemName) const;
  void addItemTemplate(const model::ItemTemplate& itemTemplate);
  const model::CharacterTemplate& getCharacterTemplate(std::string_view templateName) const;
  void addCharacterTemplate(const model::CharacterTemplate& characterTemplate);
  const model::GameEvent& getGameEvent(std::string_view eventId) const;
  void addGameEvent(const model::GameEvent& gameEvent);
  void load();
};

} // namespace db