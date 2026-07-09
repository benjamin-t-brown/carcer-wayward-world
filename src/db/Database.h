#pragma once

#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/Abilities.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/Maps.h"
#include "model/templates/SpecialEvents.h"
#include "model/templates/StatusEffects.h"
#include <stdexcept>
#include <string_view>

namespace db {

template <typename V>
const V& mapGet(const bmin::Map<bmin::String, V>& map, std::string_view key, const char* notFoundMsg) {
  const bmin::String mapKey(key.data(), key.size());
  if (!map.contains(mapKey)) {
    throw std::runtime_error((bmin::String(notFoundMsg) + mapKey.cStr()).cStr());
  }
  return (*const_cast<bmin::Map<bmin::String, V>&>(map).find(mapKey)).value;
}

class Database {
private:
  bmin::Map<bmin::String, model::ItemTemplate> itemTemplates;
  bmin::Map<bmin::String, model::CharacterTemplate> characterTemplates;
  bmin::Map<bmin::String, model::AbilityTemplate> abilityTemplates;
  bmin::Map<bmin::String, model::StatusEffectTemplate> statusEffectTemplates;
  bmin::Map<bmin::String, model::GameEvent> gameEvents;
  bmin::Map<bmin::String, model::CarcerMapTemplate> mapTemplates;

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
  const model::CarcerMapTemplate& getMapTemplate(std::string_view mapName) const;
  void addMapTemplate(const model::CarcerMapTemplate& mapTemplate);
  void load();
  void validateCombatReferences() const;
};

} // namespace db
