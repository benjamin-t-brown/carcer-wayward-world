#include "LoadAbilityTemplates.h"
#include "LoadAbilityJson.h"
#include "lib/json.hpp"
#include "lib/sdl2w/AssetLoader.h"
#include <stdexcept>

namespace db {

void loadAbilityTemplates(
    const std::string& abilitiesFilePath,
    std::unordered_map<std::string, model::AbilityTemplate>& abilityTemplates) {
  std::string fileContent =
      std::string(sdl2w::loadFileAsString(abilitiesFilePath).sliceView());

  nlohmann::json jsonData;
  try {
    jsonData = nlohmann::json::parse(fileContent, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Failed to parse JSON file " + abilitiesFilePath + ": " +
                             e.what());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of abilities");
  }

  for (const auto& abilityJson : jsonData) {
    model::AbilityTemplate abilityTemplate;

    if (!abilityJson.contains("name") || !abilityJson["name"].is_string()) {
      throw std::runtime_error("Ability missing required field: name");
    }
    abilityTemplate.name = abilityJson["name"];

    if (!abilityJson.contains("label") || !abilityJson["label"].is_string()) {
      throw std::runtime_error("Ability missing required field: label");
    }
    abilityTemplate.label = abilityJson["label"];

    if (!abilityJson.contains("description") || !abilityJson["description"].is_string()) {
      throw std::runtime_error("Ability missing required field: description");
    }
    abilityTemplate.description = abilityJson["description"];

    if (!abilityJson.contains("icon") || !abilityJson["icon"].is_string()) {
      throw std::runtime_error("Ability missing required field: icon");
    }
    abilityTemplate.icon = abilityJson["icon"];

    if (!abilityJson.contains("type") || !abilityJson["type"].is_string()) {
      throw std::runtime_error("Ability missing required field: type");
    }
    abilityTemplate.type = model::abilityTypeFromString(abilityJson["type"]);

    if (!abilityJson.contains("targetSelect") ||
        !abilityJson["targetSelect"].is_object()) {
      throw std::runtime_error("Ability missing required field: targetSelect");
    }
    abilityTemplate.targetSelect = parseTargetSelectInfo(abilityJson["targetSelect"]);

    if (!abilityJson.contains("apCost") || !abilityJson["apCost"].is_number_integer()) {
      throw std::runtime_error("Ability missing required field: apCost");
    }
    abilityTemplate.apCost = abilityJson["apCost"];

    if (!abilityJson.contains("costType") || !abilityJson["costType"].is_string()) {
      throw std::runtime_error("Ability missing required field: costType");
    }
    abilityTemplate.costType = model::abilityCostTypeFromString(abilityJson["costType"]);

    if (!abilityJson.contains("costValue") ||
        !abilityJson["costValue"].is_number_integer()) {
      throw std::runtime_error("Ability missing required field: costValue");
    }
    abilityTemplate.costValue = abilityJson["costValue"];

    if (!abilityJson.contains("depiction") || !abilityJson["depiction"].is_object()) {
      throw std::runtime_error("Ability missing required field: depiction");
    }
    abilityTemplate.depiction = parseAbilityDepiction(abilityJson["depiction"]);

    if (abilityJson.contains("attacks") && abilityJson["attacks"].is_array()) {
      for (const auto& attackJson : abilityJson["attacks"]) {
        abilityTemplate.attacks.push_back(parseAbilityAttack(attackJson));
      }
    }

    if (abilityJson.contains("statuses") && abilityJson["statuses"].is_array()) {
      for (const auto& statusJson : abilityJson["statuses"]) {
        abilityTemplate.statuses.push_back(parseAbilityStatus(statusJson));
      }
    }

    if (abilityJson.contains("restores") && abilityJson["restores"].is_array()) {
      for (const auto& restoreJson : abilityJson["restores"]) {
        abilityTemplate.restores.push_back(parseAbilityRestore(restoreJson));
      }
    }

    if (abilityTemplates.find(abilityTemplate.name) != abilityTemplates.end()) {
      throw std::runtime_error("Ability template already exists: " +
                               abilityTemplate.name);
    }

    abilityTemplates[abilityTemplate.name] = abilityTemplate;
  }
}

} // namespace db
