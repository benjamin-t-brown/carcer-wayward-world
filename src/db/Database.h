#pragma once

#include "model/Items.h"
#include "model/Character.h"
#include "model/SpecialEvents.h"
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