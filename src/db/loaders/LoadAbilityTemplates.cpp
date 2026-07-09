#include "LoadAbilityTemplates.h"
#include "LoadAbilityJson.h"
#include "lib/Json.h"
#include "lib/sdl2w/AssetLoader.h"
#include <stdexcept>

namespace db {

void loadAbilityTemplates(
    const String& abilitiesFilePath,
    bmin::Map<String, model::AbilityTemplate>& abilityTemplates) {
  const String fileContent = sdl2w::loadFileAsString(bmin::toStringView(abilitiesFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((String("Failed to parse JSON file ") + abilitiesFilePath.cStr() +
                              ": " + e.what())
                                 .cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of abilities");
  }

  for (const auto& abilityJson : jsonData) {
    model::AbilityTemplate abilityTemplate;

    if (!abilityJson.contains("name") || !abilityJson["name"].is_string()) {
      throw std::runtime_error("Ability missing required field: name");
    }
    abilityTemplate.name = abilityJson["name"].get<String>();

    if (!abilityJson.contains("label") || !abilityJson["label"].is_string()) {
      throw std::runtime_error("Ability missing required field: label");
    }
    abilityTemplate.label = abilityJson["label"].get<String>();

    if (!abilityJson.contains("description") || !abilityJson["description"].is_string()) {
      throw std::runtime_error("Ability missing required field: description");
    }
    abilityTemplate.description = abilityJson["description"].get<String>();

    if (!abilityJson.contains("icon") || !abilityJson["icon"].is_string()) {
      throw std::runtime_error("Ability missing required field: icon");
    }
    abilityTemplate.icon = abilityJson["icon"].get<String>();

    if (!abilityJson.contains("type") || !abilityJson["type"].is_string()) {
      throw std::runtime_error("Ability missing required field: type");
    }
    abilityTemplate.type = model::abilityTypeFromString(abilityJson["type"].get<String>());

    if (!abilityJson.contains("targetSelect") ||
        !abilityJson["targetSelect"].is_object()) {
      throw std::runtime_error("Ability missing required field: targetSelect");
    }
    abilityTemplate.targetSelect = parseTargetSelectInfo(abilityJson["targetSelect"]);

    if (!abilityJson.contains("apCost") || !abilityJson["apCost"].is_number_integer()) {
      throw std::runtime_error("Ability missing required field: apCost");
    }
    abilityTemplate.apCost = abilityJson["apCost"].get<int>();

    if (!abilityJson.contains("costType") || !abilityJson["costType"].is_string()) {
      throw std::runtime_error("Ability missing required field: costType");
    }
    abilityTemplate.costType = model::abilityCostTypeFromString(abilityJson["costType"].get<String>());

    if (!abilityJson.contains("costValue") ||
        !abilityJson["costValue"].is_number_integer()) {
      throw std::runtime_error("Ability missing required field: costValue");
    }
    abilityTemplate.costValue = abilityJson["costValue"].get<int>();

    if (!abilityJson.contains("depiction") || !abilityJson["depiction"].is_object()) {
      throw std::runtime_error("Ability missing required field: depiction");
    }
    abilityTemplate.depiction = parseAbilityDepiction(abilityJson["depiction"]);

    if (abilityJson.contains("attacks") && abilityJson["attacks"].is_array()) {
      for (const auto& attackJson : abilityJson["attacks"]) {
        abilityTemplate.attacks.pushBack(parseAbilityAttack(attackJson));
      }
    }

    if (abilityJson.contains("statuses") && abilityJson["statuses"].is_array()) {
      for (const auto& statusJson : abilityJson["statuses"]) {
        abilityTemplate.statuses.pushBack(parseAbilityStatus(statusJson));
      }
    }

    if (abilityJson.contains("restores") && abilityJson["restores"].is_array()) {
      for (const auto& restoreJson : abilityJson["restores"]) {
        abilityTemplate.restores.pushBack(parseAbilityRestore(restoreJson));
      }
    }

    if (abilityTemplates.contains(abilityTemplate.name)) {
      throw std::runtime_error((String("Ability template already exists: ") +
                                abilityTemplate.name)
                                   .cStr());
    }

    abilityTemplates[abilityTemplate.name] = abilityTemplate;
  }
}

} // namespace db
