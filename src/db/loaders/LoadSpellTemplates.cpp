#include "LoadSpellTemplates.h"
#include "bmin/StringInterop.h"
#include "lib/Json.h"
#include "model/templates/RuneTypes.h"
#include "sdl2w/AssetLoader.h"
#include <stdexcept>

namespace db {

void loadSpellTemplates(
    const bmin::String& spellsFilePath,
    bmin::Map<bmin::String, model::SpellTemplate>& spellTemplates) {
  const bmin::String fileContent = sdl2w::loadFileAsString(bmin::toStringView(spellsFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((bmin::String("Failed to parse JSON file ") + spellsFilePath.cStr() +
                              ": " + e.what())
                                 .cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of spells");
  }

  for (const auto& spellJson : jsonData) {
    model::SpellTemplate spellTemplate;

    if (!spellJson.contains("name") || !spellJson["name"].is_string()) {
      throw std::runtime_error("Spell missing required field: name");
    }
    spellTemplate.name = spellJson["name"].get<bmin::String>();

    if (!spellJson.contains("label") || !spellJson["label"].is_string()) {
      throw std::runtime_error("Spell missing required field: label");
    }
    spellTemplate.label = spellJson["label"].get<bmin::String>();

    if (!spellJson.contains("description") || !spellJson["description"].is_string()) {
      throw std::runtime_error("Spell missing required field: description");
    }
    spellTemplate.description = spellJson["description"].get<bmin::String>();

    if (!spellJson.contains("icon") || !spellJson["icon"].is_string()) {
      throw std::runtime_error("Spell missing required field: icon");
    }
    spellTemplate.icon = spellJson["icon"].get<bmin::String>();

    if (!spellJson.contains("abilityName") || !spellJson["abilityName"].is_string()) {
      throw std::runtime_error("Spell missing required field: abilityName");
    }
    spellTemplate.abilityName = spellJson["abilityName"].get<bmin::String>();

    if (!spellJson.contains("requiredRunes") || !spellJson["requiredRunes"].is_array()) {
      throw std::runtime_error("Spell missing required field: requiredRunes");
    }
    for (const auto& runeJson : spellJson["requiredRunes"]) {
      if (!runeJson.is_object()) {
        throw std::runtime_error(
            (bmin::String("Spell requiredRunes entry must be an object: ") + spellTemplate.name)
                .cStr());
      }
      if (!runeJson.contains("type") || !runeJson["type"].is_string()) {
        throw std::runtime_error(
            (bmin::String("Spell requiredRunes entry missing type: ") + spellTemplate.name)
                .cStr());
      }
      if (!runeJson.contains("count") || !runeJson["count"].is_number_integer()) {
        throw std::runtime_error(
            (bmin::String("Spell requiredRunes entry missing count: ") + spellTemplate.name)
                .cStr());
      }

      model::SpellRuneRequirement requirement;
      try {
        requirement.type = model::runeTypeFromString(runeJson["type"].get<bmin::String>());
      } catch (const std::runtime_error&) {
        throw std::runtime_error(
            (bmin::String("Spell requiredRunes has invalid RuneType: ") + spellTemplate.name)
                .cStr());
      }
      requirement.count = runeJson["count"].get<int>();
      if (requirement.count <= 0) {
        throw std::runtime_error(
            (bmin::String("Spell requiredRunes count must be > 0: ") + spellTemplate.name).cStr());
      }

      for (size_t i = 0; i < spellTemplate.requiredRunes.size(); ++i) {
        if (spellTemplate.requiredRunes[i].type == requirement.type) {
          throw std::runtime_error(
              (bmin::String("Spell requiredRunes has duplicate RuneType: ") + spellTemplate.name)
                  .cStr());
        }
      }

      spellTemplate.requiredRunes.pushBack(requirement);
    }

    if (spellTemplates.contains(spellTemplate.name)) {
      throw std::runtime_error((bmin::String("Spell template already exists: ") +
                                spellTemplate.name)
                                   .cStr());
    }

    spellTemplates[spellTemplate.name] = spellTemplate;
  }
}

} // namespace db
