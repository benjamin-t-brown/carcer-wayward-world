#include "Database.h"
#include "sdl2w/Logger.h"
#include "loaders/LoadAbilityTemplates.h"
#include "loaders/LoadCharacterTemplates.h"
#include "loaders/LoadItemTemplates.h"
#include "loaders/LoadMapTemplates.h"
#include "loaders/LoadSpecialEvents.h"
#include "loaders/LoadStatusEffectTemplates.h"
#include <stdexcept>

namespace db {

Database::Database() {}

void Database::validateCombatReferences() const {
  auto& abilities = const_cast<decltype(abilityTemplates)&>(abilityTemplates);
  for (auto it = abilities.begin(); it != abilities.end(); ++it) {
    const bmin::String& abilityName = (*it).key;
    const auto& abilityTemplate = (*it).value;
    for (const auto& status : abilityTemplate.statuses) {
      if (!statusEffectTemplates.contains(status.statusEffect)) {
        throw std::runtime_error(
            (bmin::String("Ability ") + abilityName.cStr() + " references unknown status effect: " +
             status.statusEffect.cStr())
                .cStr());
      }
    }
  }

  auto& statusEffects = const_cast<decltype(statusEffectTemplates)&>(statusEffectTemplates);
  for (auto it = statusEffects.begin(); it != statusEffects.end(); ++it) {
    const bmin::String& statusName = (*it).key;
    const auto& statusTemplate = (*it).value;
    for (const auto& invoked : statusTemplate.actions) {
      if (!abilityTemplates.contains(invoked.abilityName)) {
        throw std::runtime_error(
            (bmin::String("Status effect ") + statusName.cStr() + " references unknown ability: " +
             invoked.abilityName.cStr())
                .cStr());
      }
    }
  }

  auto& items = const_cast<decltype(itemTemplates)&>(itemTemplates);
  for (auto it = items.begin(); it != items.end(); ++it) {
    const bmin::String& itemName = (*it).key;
    const auto& itemTemplate = (*it).value;
    for (const auto& statusName : itemTemplate.statusEffectNames) {
      if (!statusEffectTemplates.contains(statusName)) {
        throw std::runtime_error(
            (bmin::String("Item ") + itemName.cStr() + " references unknown status effect: " +
             statusName.cStr())
                .cStr());
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
  loadSpecialEvents("assets/db/special-events.json", gameEvents);
  validateCombatReferences();
  LOG(INFO) << "Loaded database." << LOG_ENDL;
}

const model::ItemTemplate& Database::getItemTemplate(std::string_view itemName) const {
  return mapGet(itemTemplates, itemName, "Item template not found: ");
}

void Database::addItemTemplate(const model::ItemTemplate& itemTemplate) {
  itemTemplates[itemTemplate.name] = itemTemplate;
}

const model::CharacterTemplate&
Database::getCharacterTemplate(std::string_view templateName) const {
  return mapGet(characterTemplates, templateName, "Character template not found: ");
}

void Database::addCharacterTemplate(const model::CharacterTemplate& characterTemplate) {
  characterTemplates[characterTemplate.name] = characterTemplate;
}

const model::AbilityTemplate& Database::getAbilityTemplate(std::string_view abilityName) const {
  return mapGet(abilityTemplates, abilityName, "Ability template not found: ");
}

void Database::addAbilityTemplate(const model::AbilityTemplate& abilityTemplate) {
  abilityTemplates[abilityTemplate.name] = abilityTemplate;
}

const model::StatusEffectTemplate&
Database::getStatusEffectTemplate(std::string_view statusName) const {
  return mapGet(statusEffectTemplates, statusName, "Status effect template not found: ");
}

void Database::addStatusEffectTemplate(const model::StatusEffectTemplate& statusEffectTemplate) {
  statusEffectTemplates[statusEffectTemplate.name] = statusEffectTemplate;
}

const model::GameEvent& Database::getGameEvent(std::string_view eventId) const {
  return mapGet(gameEvents, eventId, "Game event not found: ");
}

void Database::addGameEvent(const model::GameEvent& gameEvent) {
  gameEvents[gameEvent.id] = gameEvent;
}

const model::CarcerMapTemplate& Database::getMapTemplate(std::string_view mapName) const {
  return mapGet(mapTemplates, mapName, "Map template not found: ");
}

void Database::addMapTemplate(const model::CarcerMapTemplate& mapTemplate) {
  mapTemplates[mapTemplate.name] = mapTemplate;
}

} // namespace db
