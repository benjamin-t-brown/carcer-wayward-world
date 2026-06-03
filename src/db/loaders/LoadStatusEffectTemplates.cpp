#include "LoadStatusEffectTemplates.h"
#include "LoadCombatJson.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/json.hpp"
#include <stdexcept>

namespace db {

void loadStatusEffectTemplates(
    const std::string& statusEffectsFilePath,
    std::unordered_map<std::string, model::StatusEffectTemplate>& statusEffectTemplates) {
  std::string fileContent = sdl2w::loadFileAsString(statusEffectsFilePath);

  nlohmann::json jsonData;
  try {
    jsonData = nlohmann::json::parse(fileContent, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Failed to parse JSON file " + statusEffectsFilePath + ": " +
                           e.what());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of status effects");
  }

  for (const auto& statusJson : jsonData) {
    model::StatusEffectTemplate statusTemplate;

    if (!statusJson.contains("name") || !statusJson["name"].is_string()) {
      throw std::runtime_error("Status effect missing required field: name");
    }
    statusTemplate.name = statusJson["name"];

    if (!statusJson.contains("description") || !statusJson["description"].is_string()) {
      throw std::runtime_error("Status effect missing required field: description");
    }
    statusTemplate.description = statusJson["description"];

    if (!statusJson.contains("duration") || !statusJson["duration"].is_number_integer()) {
      throw std::runtime_error("Status effect missing required field: duration");
    }
    statusTemplate.duration = statusJson["duration"];

    if (!statusJson.contains("events") || !statusJson["events"].is_array()) {
      throw std::runtime_error("Status effect missing required field: events");
    }
    for (const auto& eventJson : statusJson["events"]) {
      model::StatusEffectEvent event;
      if (!eventJson.contains("type") || !eventJson["type"].is_string()) {
        throw std::runtime_error("StatusEffectEvent missing type");
      }
      event.type = model::statusEventTypeFromString(eventJson["type"]);
      if (!eventJson.contains("condition") || !eventJson["condition"].is_string()) {
        throw std::runtime_error("StatusEffectEvent missing condition");
      }
      event.condition = model::statusEffectConditionFromString(eventJson["condition"]);
      statusTemplate.events.push_back(event);
    }

    if (statusJson.contains("applyBonuses") && statusJson["applyBonuses"].is_object()) {
      statusTemplate.applyBonuses = parseStats(statusJson["applyBonuses"]);
    }

    if (statusJson.contains("applyCurrentStatChange") &&
        statusJson["applyCurrentStatChange"].is_object()) {
      statusTemplate.applyCurrentStatChange = parseCurrentStats(statusJson["applyCurrentStatChange"]);
    }

    if (statusJson.contains("applyResistances") && statusJson["applyResistances"].is_array()) {
      for (const auto& resistanceJson : statusJson["applyResistances"]) {
        statusTemplate.applyResistances.push_back(parseResistance(resistanceJson));
      }
    }

    if (statusJson.contains("targetInfo") && statusJson["targetInfo"].is_object()) {
      statusTemplate.targetInfo = parseTargetSelectInfo(statusJson["targetInfo"]);
    }

    if (statusJson.contains("actions") && statusJson["actions"].is_array()) {
      for (const auto& actionJson : statusJson["actions"]) {
        model::StatusEffectAction action;
        if (!actionJson.contains("statusActionTargetType") ||
            !actionJson["statusActionTargetType"].is_string()) {
          throw std::runtime_error("StatusEffectAction missing statusActionTargetType");
        }
        action.statusActionTargetType =
            model::statusActionTargetTypeFromString(actionJson["statusActionTargetType"]);
        if (!actionJson.contains("abilityName") || !actionJson["abilityName"].is_string()) {
          throw std::runtime_error("StatusEffectAction missing abilityName");
        }
        action.abilityName = actionJson["abilityName"];
        statusTemplate.actions.push_back(action);
      }
    }

    if (statusEffectTemplates.find(statusTemplate.name) != statusEffectTemplates.end()) {
      throw std::runtime_error("Status effect template already exists: " + statusTemplate.name);
    }

    statusEffectTemplates[statusTemplate.name] = statusTemplate;
  }
}

} // namespace db
