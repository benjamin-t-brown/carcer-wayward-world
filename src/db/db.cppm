module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <string>
#include <stdexcept>

export module carcer.db;
export import bmin.containers;
export import carcer.model.templates.Abilities;
export import carcer.model.templates.CharacterTemplate;
export import carcer.model.templates.Items;
export import carcer.model.templates.MapGrids;
export import carcer.model.templates.Maps;
export import carcer.model.templates.SpecialEvents;
export import carcer.model.templates.Spells;
export import carcer.model.templates.StatusEffects;
export import carcer.model.templates.Tileset;
export import carcer.lib.Json;
export import carcer.model.templates.AbilityTypes;
import bmin.string_interop;

export {

// --- from db/Database.h ---
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
  bmin::Map<bmin::String, model::SpellTemplate> spellTemplates;
  bmin::Map<bmin::String, model::StatusEffectTemplate> statusEffectTemplates;
  bmin::Map<bmin::String, model::GameEvent> gameEvents;
  bmin::Map<bmin::String, model::CarcerMapTemplate> mapTemplates;
  bmin::Map<bmin::String, model::MapGridTemplate> mapGridTemplates;
  bmin::Map<bmin::String, model::TilesetTemplate> tilesetTemplates;

public:
  Database();
  ~Database() = default;

  const model::ItemTemplate& getItemTemplate(std::string_view itemName) const;
  void addItemTemplate(const model::ItemTemplate& itemTemplate);
  const model::CharacterTemplate& getCharacterTemplate(std::string_view templateName) const;
  void addCharacterTemplate(const model::CharacterTemplate& characterTemplate);
  const model::AbilityTemplate& getAbilityTemplate(std::string_view abilityName) const;
  const model::AbilityTemplate* findAbilityTemplate(std::string_view abilityName) const;
  void addAbilityTemplate(const model::AbilityTemplate& abilityTemplate);
  const model::SpellTemplate& getSpellTemplate(std::string_view spellName) const;
  const model::SpellTemplate* findSpellTemplate(std::string_view spellName) const;
  void addSpellTemplate(const model::SpellTemplate& spellTemplate);
  const model::StatusEffectTemplate& getStatusEffectTemplate(std::string_view statusName) const;
  void addStatusEffectTemplate(const model::StatusEffectTemplate& statusEffectTemplate);
  const model::GameEvent& getGameEvent(std::string_view eventId) const;
  const bmin::Map<bmin::String, model::GameEvent>& getGameEvents() const;
  void addGameEvent(const model::GameEvent& gameEvent);
  const model::CarcerMapTemplate& getMapTemplate(std::string_view mapName) const;
  void addMapTemplate(const model::CarcerMapTemplate& mapTemplate);
  const bmin::Map<bmin::String, model::CarcerMapTemplate>& getMapTemplates() const;
  const model::MapGridTemplate& getMapGridTemplate(std::string_view gridName) const;
  const model::MapGridTemplate* findMapGridTemplate(std::string_view gridName) const;
  const bmin::Map<bmin::String, model::MapGridTemplate>& getMapGridTemplates() const;
  void addMapGridTemplate(const model::MapGridTemplate& mapGridTemplate);
  const model::TilesetTemplate& getTilesetTemplate(std::string_view tilesetName) const;
  const model::TilesetTemplate* findTilesetTemplate(std::string_view tilesetName) const;
  void addTilesetTemplate(const model::TilesetTemplate& tilesetTemplate);
  void load();
  void validateCombatReferences() const;
};

} // namespace db

// --- from db/loaders/LoadAbilityJson.h ---
namespace db {

bmin::DynArray<model::Dice> parseDiceArray(const Json& json, const bmin::String& fieldName);
model::TargetSelectInfo parseTargetSelectInfo(const Json& json);
model::Stats parseStats(const Json& json);
model::CurrentStats parseCurrentStats(const Json& json);
model::Resistance parseResistance(const Json& json);
model::AbilitySave parseAbilitySave(const Json& json);
model::AbilityAttackDmg parseAbilityAttackDmg(const Json& json);
model::AbilityAttack parseAbilityAttack(const Json& json);
model::AbilityStatus parseAbilityStatus(const Json& json);
model::AbilityRestore parseAbilityRestore(const Json& json);
model::AbilityDamage parseAbilityDamage(const Json& json);
model::AbilityDepiction parseAbilityDepiction(const Json& json);

} // namespace db

// --- from db/loaders/LoadAbilityTemplates.h ---
namespace db {

void loadAbilityTemplates(const bmin::String& abilitiesFilePath,
                          bmin::Map<bmin::String, model::AbilityTemplate>& abilityTemplates);

} // namespace db

// --- from db/loaders/LoadCharacterTemplates.h ---
namespace db {

void loadCharacterTemplates(
    const bmin::String& charactersFilePath,
    bmin::Map<bmin::String, model::CharacterTemplate>& characterTemplates);

} // namespace db

// --- from db/loaders/LoadItemTemplates.h ---
namespace db {

void loadItemTemplates(const bmin::String& itemsFilePath,
                       bmin::Map<bmin::String, model::ItemTemplate>& itemTemplates);

} // namespace db

// --- from db/loaders/LoadMapGridTemplates.h ---
namespace db {

void loadMapGridTemplates(const bmin::String& mapGridsFilePath,
                          bmin::Map<bmin::String, model::MapGridTemplate>& mapGridTemplates);

} // namespace db

// --- from db/loaders/LoadMapTemplates.h ---
namespace db {

void loadMapTemplates(const bmin::String& mapsFilePath,
                      bmin::Map<bmin::String, model::CarcerMapTemplate>& mapTemplates);

} // namespace db

// --- from db/loaders/LoadSpecialEvents.h ---
namespace db {

void loadSpecialEvents(const bmin::String& specialEventsFilePath,
                       bmin::Map<bmin::String, model::GameEvent>& specialEvents);

void loadSpecialEvents(const bmin::String& specialEventsFilePath,
                       bmin::Map<bmin::String, model::GameEvent>& specialEvents,
                       bmin::DynArray<bmin::String>& eventsToLoad);

} // namespace db

// --- from db/loaders/LoadSpellTemplates.h ---
namespace db {

void loadSpellTemplates(const bmin::String& spellsFilePath,
                        bmin::Map<bmin::String, model::SpellTemplate>& spellTemplates);

} // namespace db

// --- from db/loaders/LoadStatusEffectTemplates.h ---
namespace db {

void loadStatusEffectTemplates(
    const bmin::String& statusEffectsFilePath,
    bmin::Map<bmin::String, model::StatusEffectTemplate>& statusEffectTemplates);

} // namespace db

// --- from db/loaders/LoadTilesetTemplates.h ---
namespace db {

void loadTilesetTemplates(const bmin::String& tilesetsFilePath,
                          bmin::Map<bmin::String, model::TilesetTemplate>& tilesetTemplates);

} // namespace db

} // export
