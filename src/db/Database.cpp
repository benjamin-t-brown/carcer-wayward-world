#include "Database.h"
#include "lib/sdl2w/Logger.h"
#include "loaders/LoadAbilityTemplates.h"
#include "loaders/LoadCharacterTemplates.h"
#include "loaders/LoadMapTemplates.h"
#include "loaders/LoadItemTemplates.h"
#include "loaders/LoadStatusEffectTemplates.h"
#include <stdexcept>
#include <string>

namespace db {

Database::Database() {}

void Database::validateCombatReferences() const {
  for (const auto& [abilityName, abilityTemplate] : abilityTemplates) {
    for (const auto& status : abilityTemplate.statuses) {
      if (statusEffectTemplates.find(status.statusEffect) == statusEffectTemplates.end()) {
        throw std::runtime_error("Ability " + abilityName + " references unknown status effect: " +
                                 status.statusEffect);
      }
    }
  }

  for (const auto& [statusName, statusTemplate] : statusEffectTemplates) {
    for (const auto& invoked : statusTemplate.actions) {
      if (abilityTemplates.find(invoked.abilityName) == abilityTemplates.end()) {
        throw std::runtime_error("Status effect " + statusName +
                                 " references unknown ability: " + invoked.abilityName);
      }
    }
  }

  for (const auto& [itemName, itemTemplate] : itemTemplates) {
    for (const auto& statusName : itemTemplate.statusEffectNames) {
      if (statusEffectTemplates.find(statusName) == statusEffectTemplates.end()) {
        throw std::runtime_error("Item " + itemName + " references unknown status effect: " +
                                 statusName);
      }
    }
  }
}

void Database::load() {
  LOG(INFO) << "Loading database..." << LOG_ENDL;
  loadStatusEffectTemplates("assets/db/status-effects.json", statusEffectTemplates);
  loadAbilityTemplates("assets/db/abilities.json", abilityTemplates);
  loadItemTemplates("assets/db/items.json", itemTemplates);
  loadCharacterTemplates("assets/db/characters.json", characterTemplates);
  loadMapTemplates("assets/db/maps.json", mapTemplates);
  validateCombatReferences();
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

const model::CharacterTemplate&
Database::getCharacterTemplate(std::string_view templateName) const {
  if (characterTemplates.find(std::string(templateName)) == characterTemplates.end()) {
    throw std::runtime_error("Character template not found: " +
                             std::string(templateName));
  }
  return characterTemplates.at(std::string(templateName));
}

void Database::addCharacterTemplate(const model::CharacterTemplate& characterTemplate) {
  characterTemplates[characterTemplate.name] = characterTemplate;
}

const model::AbilityTemplate& Database::getAbilityTemplate(std::string_view abilityName) const {
  if (abilityTemplates.find(std::string(abilityName)) == abilityTemplates.end()) {
    throw std::runtime_error("Ability template not found: " + std::string(abilityName));
  }
  return abilityTemplates.at(std::string(abilityName));
}

void Database::addAbilityTemplate(const model::AbilityTemplate& abilityTemplate) {
  abilityTemplates[abilityTemplate.name] = abilityTemplate;
}

const model::StatusEffectTemplate&
Database::getStatusEffectTemplate(std::string_view statusName) const {
  if (statusEffectTemplates.find(std::string(statusName)) == statusEffectTemplates.end()) {
    throw std::runtime_error("Status effect template not found: " + std::string(statusName));
  }
  return statusEffectTemplates.at(std::string(statusName));
}

void Database::addStatusEffectTemplate(const model::StatusEffectTemplate& statusEffectTemplate) {
  statusEffectTemplates[statusEffectTemplate.name] = statusEffectTemplate;
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

const model::CarcerMapTemplate& Database::getMapTemplate(std::string_view mapName) const {
  if (mapTemplates.find(std::string(mapName)) == mapTemplates.end()) {
    throw std::runtime_error("Map template not found: " + std::string(mapName));
  }
  return mapTemplates.at(std::string(mapName));
}

void Database::addMapTemplate(const model::CarcerMapTemplate& mapTemplate) {
  mapTemplates[mapTemplate.name] = mapTemplate;
}

} // namespace db
