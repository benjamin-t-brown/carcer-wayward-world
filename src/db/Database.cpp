module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <string>
#include <stdexcept>

module carcer.db;
import carcer.data;
import bmin.containers;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace db {

Database::Database() {}

void Database::validateCombatReferences() const {
  for (auto it = abilityTemplates.begin(); it != abilityTemplates.end(); ++it) {
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

  for (auto it = statusEffectTemplates.begin(); it != statusEffectTemplates.end(); ++it) {
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

  for (auto it = itemTemplates.begin(); it != itemTemplates.end(); ++it) {
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
    if (itemTemplate.itemType == model::ItemType::RUNE) {
      if (!itemTemplate.runeType.has_value()) {
        throw std::runtime_error(
            (bmin::String("RUNE item missing required runeType: ") + itemName.cStr()).cStr());
      }
    } else if (itemTemplate.runeType.has_value()) {
      throw std::runtime_error(
          (bmin::String("Non-RUNE item must not set runeType: ") + itemName.cStr()).cStr());
    }
  }

  for (auto it = spellTemplates.begin(); it != spellTemplates.end(); ++it) {
    const bmin::String& spellName = (*it).key;
    const auto& spellTemplate = (*it).value;
    if (!abilityTemplates.contains(spellTemplate.abilityName)) {
      throw std::runtime_error(
          (bmin::String("Spell ") + spellName.cStr() + " references unknown ability: " +
           spellTemplate.abilityName.cStr())
              .cStr());
    }
    for (const auto& requirement : spellTemplate.requiredRunes) {
      try {
        (void)model::runeTypeIndex(requirement.type);
      } catch (const std::runtime_error&) {
        throw std::runtime_error(
            (bmin::String("Spell ") + spellName.cStr() + " requiredRunes has invalid RuneType")
                .cStr());
      }
      if (requirement.count <= 0) {
        throw std::runtime_error(
            (bmin::String("Spell ") + spellName.cStr() + " requiredRunes count must be > 0")
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
  loadSpellTemplates("assets/db/spells.json", spellTemplates);
  loadCharacterTemplates("assets/db/characters.json", characterTemplates);
  loadMapTemplates("assets/db/maps.json", mapTemplates);
  loadMapGridTemplates("assets/db/map-grids.json", mapGridTemplates);
  loadTilesetTemplates("assets/db/tilesets.json", tilesetTemplates);
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

const model::AbilityTemplate* Database::findAbilityTemplate(std::string_view abilityName) const {
  const auto mapKey = bmin::String(abilityName.data(), abilityName.size());
  auto it = abilityTemplates.find(mapKey);
  if (it == abilityTemplates.end()) {
    return nullptr;
  }
  return &(*it).value;
}

void Database::addAbilityTemplate(const model::AbilityTemplate& abilityTemplate) {
  abilityTemplates[abilityTemplate.name] = abilityTemplate;
}

const model::SpellTemplate& Database::getSpellTemplate(std::string_view spellName) const {
  return mapGet(spellTemplates, spellName, "Spell template not found: ");
}

const model::SpellTemplate* Database::findSpellTemplate(std::string_view spellName) const {
  const auto mapKey = bmin::String(spellName.data(), spellName.size());
  auto it = spellTemplates.find(mapKey);
  if (it == spellTemplates.end()) {
    return nullptr;
  }
  return &(*it).value;
}

void Database::addSpellTemplate(const model::SpellTemplate& spellTemplate) {
  spellTemplates[spellTemplate.name] = spellTemplate;
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

const bmin::Map<bmin::String, model::GameEvent>& Database::getGameEvents() const {
  return gameEvents;
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

const bmin::Map<bmin::String, model::CarcerMapTemplate>& Database::getMapTemplates() const {
  return mapTemplates;
}

const model::MapGridTemplate& Database::getMapGridTemplate(std::string_view gridName) const {
  return mapGet(mapGridTemplates, gridName, "Map grid template not found: ");
}

const model::MapGridTemplate*
Database::findMapGridTemplate(std::string_view gridName) const {
  const auto mapKey = bmin::String(gridName.data(), gridName.size());
  auto it = mapGridTemplates.find(mapKey);
  if (it == mapGridTemplates.end()) {
    return nullptr;
  }
  return &(*it).value;
}

void Database::addMapGridTemplate(const model::MapGridTemplate& mapGridTemplate) {
  mapGridTemplates[mapGridTemplate.name] = mapGridTemplate;
}

const bmin::Map<bmin::String, model::MapGridTemplate>& Database::getMapGridTemplates() const {
  return mapGridTemplates;
}

const model::TilesetTemplate&
Database::getTilesetTemplate(std::string_view tilesetName) const {
  return mapGet(tilesetTemplates, tilesetName, "Tileset template not found: ");
}

const model::TilesetTemplate*
Database::findTilesetTemplate(std::string_view tilesetName) const {
  const auto mapKey = bmin::String(tilesetName.data(), tilesetName.size());
  auto it = tilesetTemplates.find(mapKey);
  if (it == tilesetTemplates.end()) {
    return nullptr;
  }
  return &(*it).value;
}

void Database::addTilesetTemplate(const model::TilesetTemplate& tilesetTemplate) {
  tilesetTemplates[tilesetTemplate.name] = tilesetTemplate;
}

} // namespace db
