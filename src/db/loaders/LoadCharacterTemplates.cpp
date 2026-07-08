#include "LoadCharacterTemplates.h"
#include "lib/json.hpp"
#include "lib/sdl2w/AssetLoader.h"
#include "model/stats/CharacterStats.h"
#include <set>
#include <stdexcept>

namespace {

model::CharacterTemplateType
getCharacterTemplateTypeFromString(const std::string& typeStr) {
  if (typeStr == "TOWNSPERSON") {
    return model::CharacterTemplateType::TOWNSPERSON;
  } else if (typeStr == "TOWNSPERSON_STATIC") {
    return model::CharacterTemplateType::TOWNSPERSON_STATIC;
  } else if (typeStr == "ENEMY") {
    return model::CharacterTemplateType::ENEMY;
  } else if (typeStr == "ENEMY_STATIC") {
    return model::CharacterTemplateType::ENEMY_STATIC;
  }
  throw std::runtime_error("Invalid character template type: " + typeStr);
}

model::CharacterTemplateBehaviorName
getCharacterTemplateBehaviorNameFromString(const std::string& behaviorStr) {
  if (behaviorStr == "MOVE_RANDOMLY") {
    return model::CharacterTemplateBehaviorName::MOVE_RANDOMLY;
  } else if (behaviorStr == "IMMOBILE") {
    return model::CharacterTemplateBehaviorName::IMMOBILE;
  } else if (behaviorStr == "IMMOBILE_UNTIL_ENEMY_SPOTTED") {
    return model::CharacterTemplateBehaviorName::IMMOBILE_UNTIL_ENEMY_SPOTTED;
  } else if (behaviorStr == "SEEK_MARKER") {
    return model::CharacterTemplateBehaviorName::SEEK_MARKER;
  } else if (behaviorStr == "MOVE_LEFT_RIGHT") {
    return model::CharacterTemplateBehaviorName::MOVE_LEFT_RIGHT;
  } else if (behaviorStr == "MOVE_UP_DOWN") {
    return model::CharacterTemplateBehaviorName::MOVE_UP_DOWN;
  }
  throw std::runtime_error("Invalid character template behavior: " + behaviorStr);
}

std::string getStringFromCharacterTemplateBehaviorName(
    model::CharacterTemplateBehaviorName behaviorName) {
  switch (behaviorName) {
  case model::CharacterTemplateBehaviorName::MOVE_RANDOMLY:
    return "MOVE_RANDOMLY";
  case model::CharacterTemplateBehaviorName::IMMOBILE:
    return "IMMOBILE";
  case model::CharacterTemplateBehaviorName::IMMOBILE_UNTIL_ENEMY_SPOTTED:
    return "IMMOBILE_UNTIL_ENEMY_SPOTTED";
  case model::CharacterTemplateBehaviorName::SEEK_MARKER:
    return "SEEK_MARKER";
  case model::CharacterTemplateBehaviorName::MOVE_LEFT_RIGHT:
    return "MOVE_LEFT_RIGHT";
  case model::CharacterTemplateBehaviorName::MOVE_UP_DOWN:
    return "MOVE_UP_DOWN";
  }
  throw std::runtime_error("Unknown character template behavior enum value");
}

void loadIntField(const nlohmann::json& json, const char* fieldName, int& out) {
  if (json.contains(fieldName)) {
    out = json[fieldName];
  }
}

void loadGenericCombatStats(const nlohmann::json& json,
                            model::GenericCombatStats& stats) {
  loadIntField(json, "str", stats.str);
  loadIntField(json, "mnd", stats.mnd);
  loadIntField(json, "con", stats.con);
  loadIntField(json, "agi", stats.agi);
  loadIntField(json, "lck", stats.lck);
}

void loadWeaponMasteryStats(const nlohmann::json& json,
                            model::WeaponMasteryStats& stats) {
  loadIntField(json, "edged", stats.edged);
  loadIntField(json, "pole", stats.pole);
  loadIntField(json, "blunt", stats.blunt);
  loadIntField(json, "range", stats.range);
  loadIntField(json, "unarmed", stats.unarmed);
}

void loadMagicMasteryStats(const nlohmann::json& json, model::MagicMasteryStats& stats) {
  loadIntField(json, "mana", stats.mana);
  loadIntField(json, "abilityPower", stats.abilityPower);
  loadIntField(json, "attunement", stats.attunement);
  loadIntField(json, "faith", stats.faith);
  loadIntField(json, "lore", stats.lore);
}

void loadBodyMasteryStats(const nlohmann::json& json, model::BodyMasteryStats& stats) {
  loadIntField(json, "resistPhysical", stats.resistPhysical);
  loadIntField(json, "resistMagical", stats.resistMagical);
  loadIntField(json, "healingEffectiveness", stats.healingEffectiveness);
  loadIntField(json, "dr", stats.dr);
  loadIntField(json, "armorTraining", stats.armorTraining);
}

void loadTrainableCombatStats(const nlohmann::json& json,
                              model::TrainableCombatStats& stats) {
  if (json.contains("weapon") && json["weapon"].is_object()) {
    loadWeaponMasteryStats(json["weapon"], stats.weapon);
  }
  if (json.contains("magic") && json["magic"].is_object()) {
    loadMagicMasteryStats(json["magic"], stats.magic);
  }
  if (json.contains("body") && json["body"].is_object()) {
    loadBodyMasteryStats(json["body"], stats.body);
  }
}

void loadCharacterSkills(const nlohmann::json& json, model::CharacterSkills& skills) {
  loadIntField(json, "trickery", skills.trickery);
  loadIntField(json, "stealth", skills.stealth);
  loadIntField(json, "social", skills.social);
  loadIntField(json, "magicItemUse", skills.magicItemUse);
  loadIntField(json, "cooking", skills.cooking);
  loadIntField(json, "acrobatics", skills.acrobatics);
  loadIntField(json, "survival", skills.survival);
  loadIntField(json, "focus", skills.focus);
  loadIntField(json, "conditioning", skills.conditioning);
}

void loadCharacterStats(const nlohmann::json& characterJson,
                        model::CharacterStats& stats) {
  if (characterJson.contains("stats") && characterJson["stats"].is_object()) {
    const auto& statsJson = characterJson["stats"];
    if (statsJson.contains("generic") && statsJson["generic"].is_object()) {
      loadGenericCombatStats(statsJson["generic"], stats.generic);
    }
    if (statsJson.contains("trainable") && statsJson["trainable"].is_object()) {
      loadTrainableCombatStats(statsJson["trainable"], stats.trainable);
    }
    if (statsJson.contains("skills") && statsJson["skills"].is_object()) {
      loadCharacterSkills(statsJson["skills"], stats.skills);
    }
  }

  if (characterJson.contains("combat") && characterJson["combat"].is_object()) {
    const auto& combatJson = characterJson["combat"];
    if (combatJson.contains("stats") && combatJson["stats"].is_object()) {
      loadGenericCombatStats(combatJson["stats"], stats.generic);
    }
  }
}

std::string getSpriteOffsetAsString(const nlohmann::json& characterJson) {
  if (!characterJson.contains("spriteOffset")) {
    throw std::runtime_error("Character missing required field: spriteOffset");
  }
  if (characterJson["spriteOffset"].is_number_integer()) {
    return std::to_string(characterJson["spriteOffset"].get<int>());
  }
  if (characterJson["spriteOffset"].is_string()) {
    return characterJson["spriteOffset"];
  }
  throw std::runtime_error("Character spriteOffset must be a number or string");
}

} // namespace

namespace db {

void loadCharacterTemplates(
    const std::string& charactersFilePath,
    std::unordered_map<std::string, model::CharacterTemplate>& characterTemplates) {
  std::string fileContent =
      std::string(sdl2w::loadFileAsString(charactersFilePath).sliceView());

  nlohmann::json jsonData;
  try {
    jsonData = nlohmann::json::parse(fileContent, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Failed to parse JSON file " + charactersFilePath + ": " +
                             e.what());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of characters");
  }

  for (const auto& characterJson : jsonData) {
    model::CharacterTemplate characterTemplate;

    if (!characterJson.contains("type") || !characterJson["type"].is_string()) {
      throw std::runtime_error("Character missing required field: type");
    }
    characterTemplate.type = getCharacterTemplateTypeFromString(characterJson["type"]);

    if (!characterJson.contains("name") || !characterJson["name"].is_string()) {
      throw std::runtime_error("Character missing required field: name");
    }
    characterTemplate.name = characterJson["name"];

    if (!characterJson.contains("label") || !characterJson["label"].is_string()) {
      throw std::runtime_error("Character missing required field: label");
    }
    characterTemplate.label = characterJson["label"];

    if (!characterJson.contains("spritesheet") ||
        !characterJson["spritesheet"].is_string()) {
      throw std::runtime_error("Character missing required field: spritesheet");
    }
    characterTemplate.spritesheetName = characterJson["spritesheet"];
    characterTemplate.spriteOffset = getSpriteOffsetAsString(characterJson);

    if (characterJson.contains("talk") && characterJson["talk"].is_object()) {
      const auto& talkJson = characterJson["talk"];
      if (talkJson.contains("talkName") && talkJson["talkName"].is_string()) {
        characterTemplate.talk.talkName = talkJson["talkName"];
      }
      if (talkJson.contains("portraitName") && talkJson["portraitName"].is_string()) {
        characterTemplate.talk.portraitName = talkJson["portraitName"];
      }
    }

    if (characterJson.contains("behavior") && characterJson["behavior"].is_object()) {
      const auto& behaviorJson = characterJson["behavior"];
      if (behaviorJson.contains("behaviorName") &&
          behaviorJson["behaviorName"].is_string()) {
        const std::string behaviorStr = behaviorJson["behaviorName"];
        if (!behaviorStr.empty()) {
          characterTemplate.behavior.behaviorName =
              getStringFromCharacterTemplateBehaviorName(
                  getCharacterTemplateBehaviorNameFromString(behaviorStr));
        }
      }
    }

    loadCharacterStats(characterJson, characterTemplate.stats);

    if (characterJson.contains("combat") && characterJson["combat"].is_object()) {
      const auto& combatJson = characterJson["combat"];
      if (combatJson.contains("hp")) {
        characterTemplate.combat.hp = combatJson["hp"];
      }
      if (combatJson.contains("mp")) {
        characterTemplate.combat.mp = combatJson["mp"];
      }
      if (combatJson.contains("dropTable") && combatJson["dropTable"].is_string()) {
        characterTemplate.combat.dropTable = combatJson["dropTable"];
      }
    }

    if (characterJson.contains("sound") && characterJson["sound"].is_object()) {
      const auto& soundJson = characterJson["sound"];
      if (soundJson.contains("deathSoundName") &&
          soundJson["deathSoundName"].is_string()) {
        characterTemplate.sound.deathSoundName = soundJson["deathSoundName"];
      } else if (soundJson.contains("deathSound") &&
                 soundJson["deathSound"].is_string()) {
        characterTemplate.sound.deathSoundName = soundJson["deathSound"];
      }
      if (soundJson.contains("weaponSoundName") &&
          soundJson["weaponSoundName"].is_string()) {
        characterTemplate.sound.weaponSoundName = soundJson["weaponSoundName"];
      } else if (soundJson.contains("weaponSound") &&
                 soundJson["weaponSound"].is_string()) {
        characterTemplate.sound.weaponSoundName = soundJson["weaponSound"];
      }
    }

    if (characterJson.contains("statuses") && characterJson["statuses"].is_array()) {
      for (const auto& statusJson : characterJson["statuses"]) {
        if (statusJson.contains("status") && statusJson["status"].is_string()) {
          characterTemplate.statuses.push_back({statusJson["status"]});
        }
      }
    }

    if (characterJson.contains("vision") && characterJson["vision"].is_object()) {
      const auto& visionJson = characterJson["vision"];
      if (visionJson.contains("radius")) {
        characterTemplate.vision.radius = visionJson["radius"];
      }
    }

    if (characterTemplates.find(characterTemplate.name) != characterTemplates.end()) {
      throw std::runtime_error("Character template already exists: " +
                               characterTemplate.name);
    }

    characterTemplates[characterTemplate.name] = characterTemplate;
  }
}

} // namespace db
