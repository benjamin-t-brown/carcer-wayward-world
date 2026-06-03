#pragma once

#include "model/templates/Abilities.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/StatusEffects.h"
#include "model/templates/SpecialEvents.h"
#include <unordered_map>

namespace db {

class Database {
private:
  std::unordered_map<std::string, model::ItemTemplate> itemTemplates;
  std::unordered_map<std::string, model::CharacterTemplate> characterTemplates;
  std::unordered_map<std::string, model::AbilityTemplate> abilityTemplates;
  std::unordered_map<std::string, model::StatusEffectTemplate> statusEffectTemplates;
  std::unordered_map<std::string, model::GameEvent> gameEvents;

public:
  Database();
  ~Database() = default;

  const model::ItemTemplate& getItemTemplate(std::string_view itemName) const;
  void addItemTemplate(const model::ItemTemplate& itemTemplate);
  const model::CharacterTemplate& getCharacterTemplate(std::string_view templateName) const;
  void addCharacterTemplate(const model::CharacterTemplate& characterTemplate);
  const model::AbilityTemplate& getAbilityTemplate(std::string_view abilityName) const;
  void addAbilityTemplate(const model::AbilityTemplate& abilityTemplate);
  const model::StatusEffectTemplate& getStatusEffectTemplate(std::string_view statusName) const;
  void addStatusEffectTemplate(const model::StatusEffectTemplate& statusEffectTemplate);
  const model::GameEvent& getGameEvent(std::string_view eventId) const;
  void addGameEvent(const model::GameEvent& gameEvent);
  void load();
  void validateCombatReferences() const;
};

} // namespace db
