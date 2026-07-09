#include "LoadStatusEffectTemplates.h"
#include "bmin/StringInterop.h"
#include "LoadAbilityJson.h"
#include "lib/Json.h"
#include "lib/sdl2w/AssetLoader.h"
#include <stdexcept>

namespace db {

namespace {

model::StatusEffectEvent parseStatusEffectEvent(const Json& eventJson) {
  model::StatusEffectEvent event;
  if (!eventJson.contains("type") || !eventJson["type"].is_string()) {
    throw std::runtime_error("StatusEffectEvent missing type");
  }
  event.type = model::statusEventTypeFromString(eventJson["type"].get<bmin::String>());
  if (!eventJson.contains("condition") || !eventJson["condition"].is_string()) {
    throw std::runtime_error("StatusEffectEvent missing condition");
  }
  event.condition = model::statusEffectConditionFromString(eventJson["condition"].get<bmin::String>());
  return event;
}

model::StatusEffectDurationScale
parseStatusEffectDurationScale(const Json& scaleJson) {
  model::StatusEffectDurationScale scale;
  if (!scaleJson.contains("durationStat") || !scaleJson["durationStat"].is_string()) {
    throw std::runtime_error("durationScale missing durationStat");
  }
  scale.durationStat = model::statsEnumFromString(scaleJson["durationStat"].get<bmin::String>());
  if (!scaleJson.contains("durationStatMult") ||
      !scaleJson["durationStatMult"].is_number_integer()) {
    throw std::runtime_error("durationScale missing durationStatMult");
  }
  scale.durationStatMult = scaleJson["durationStatMult"].get<int>();
  return scale;
}

} // namespace

void loadStatusEffectTemplates(
    const bmin::String& statusEffectsFilePath,
    bmin::Map<bmin::String, model::StatusEffectTemplate>& statusEffectTemplates) {
  const bmin::String fileContent = sdl2w::loadFileAsString(bmin::toStringView(statusEffectsFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((bmin::String("Failed to parse JSON file ") +
                              statusEffectsFilePath.cStr() + ": " + e.what())
                                 .cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of status effects");
  }

  for (const auto& statusJson : jsonData) {
    model::StatusEffectTemplate statusTemplate;

    if (!statusJson.contains("name") || !statusJson["name"].is_string()) {
      throw std::runtime_error("Status effect missing required field: name");
    }
    statusTemplate.name = statusJson["name"].get<bmin::String>();

    if (!statusJson.contains("description") || !statusJson["description"].is_string()) {
      throw std::runtime_error("Status effect missing required field: description");
    }
    statusTemplate.description = statusJson["description"].get<bmin::String>();

    if (statusJson.contains("duration")) {
      throw std::runtime_error(
          (bmin::String("Status effect uses legacy field 'duration'; rename to 'baseDuration': ") +
           statusTemplate.name)
              .cStr());
    }
    if (!statusJson.contains("baseDuration") ||
        !statusJson["baseDuration"].is_number_integer()) {
      throw std::runtime_error("Status effect missing required field: baseDuration");
    }
    statusTemplate.baseDuration = statusJson["baseDuration"].get<int>();

    if (statusJson.contains("events")) {
      throw std::runtime_error(
          (bmin::String("Status effect uses legacy top-level 'events'; move events onto each action: ") +
           statusTemplate.name)
              .cStr());
    }
    if (statusJson.contains("targetInfo")) {
      throw std::runtime_error(
          (bmin::String("Status effect uses legacy field 'targetInfo'; remove it: ") +
           statusTemplate.name)
              .cStr());
    }

    if (statusJson.contains("durationScale") && statusJson["durationScale"].is_object()) {
      statusTemplate.durationScale =
          parseStatusEffectDurationScale(statusJson["durationScale"]);
    }

    if (statusJson.contains("applyBonuses") && statusJson["applyBonuses"].is_object()) {
      statusTemplate.applyBonuses = parseStats(statusJson["applyBonuses"]);
    }

    if (statusJson.contains("applyCurrentStatChange") &&
        statusJson["applyCurrentStatChange"].is_object()) {
      statusTemplate.applyCurrentStatChange =
          parseCurrentStats(statusJson["applyCurrentStatChange"]);
    }

    if (statusJson.contains("applyResistances") &&
        statusJson["applyResistances"].is_array()) {
      for (const auto& resistanceJson : statusJson["applyResistances"]) {
        statusTemplate.applyResistances.pushBack(parseResistance(resistanceJson));
      }
    }

    if (statusJson.contains("actions") && statusJson["actions"].is_array()) {
      for (const auto& actionJson : statusJson["actions"]) {
        model::StatusEffectAction action;
        if (!actionJson.contains("statusActionTargetType") ||
            !actionJson["statusActionTargetType"].is_string()) {
          throw std::runtime_error("StatusEffectAction missing statusActionTargetType");
        }
        action.statusActionTargetType =
            model::statusActionTargetTypeFromString(actionJson["statusActionTargetType"].get<bmin::String>());
        if (!actionJson.contains("abilityName") ||
            !actionJson["abilityName"].is_string()) {
          throw std::runtime_error("StatusEffectAction missing abilityName");
        }
        action.abilityName = actionJson["abilityName"].get<bmin::String>();
        if (!actionJson.contains("events") || !actionJson["events"].is_array()) {
          throw std::runtime_error("StatusEffectAction missing events array");
        }
        for (const auto& eventJson : actionJson["events"]) {
          action.events.pushBack(parseStatusEffectEvent(eventJson));
        }
        statusTemplate.actions.pushBack(action);
      }
    }

    if (statusEffectTemplates.contains(statusTemplate.name)) {
      throw std::runtime_error((bmin::String("Status effect template already exists: ") +
                                statusTemplate.name)
                                   .cStr());
    }

    statusEffectTemplates[statusTemplate.name] = statusTemplate;
  }
}

} // namespace db
