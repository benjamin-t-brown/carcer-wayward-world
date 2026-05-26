#include "LoadCharacterTemplates.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/json.hpp"
#include <stdexcept>

namespace {

model::CharacterTemplateType getCharacterTemplateTypeFromString(const std::string& typeStr) {
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
  std::string fileContent = sdl2w::loadFileAsString(charactersFilePath);

  nlohmann::json jsonData;
  try {
    jsonData = nlohmann::json::parse(fileContent, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Failed to parse JSON file " + charactersFilePath + ": " + e.what());
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

    if (!characterJson.contains("spritesheet") || !characterJson["spritesheet"].is_string()) {
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
      if (behaviorJson.contains("behaviorName") && behaviorJson["behaviorName"].is_string()) {
        characterTemplate.behavior.behaviorName = behaviorJson["behaviorName"];
      }
    }

    if (characterJson.contains("combat") && characterJson["combat"].is_object()) {
      const auto& combatJson = characterJson["combat"];
      if (combatJson.contains("stats") && combatJson["stats"].is_object()) {
        const auto& statsJson = combatJson["stats"];
        if (statsJson.contains("str")) {
          characterTemplate.combat.stats.str = statsJson["str"];
        }
        if (statsJson.contains("mnd")) {
          characterTemplate.combat.stats.mnd = statsJson["mnd"];
        }
        if (statsJson.contains("con")) {
          characterTemplate.combat.stats.con = statsJson["con"];
        }
        if (statsJson.contains("agi")) {
          characterTemplate.combat.stats.agi = statsJson["agi"];
        }
        if (statsJson.contains("lck")) {
          characterTemplate.combat.stats.lck = statsJson["lck"];
        }
      }
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
      if (soundJson.contains("deathSoundName") && soundJson["deathSoundName"].is_string()) {
        characterTemplate.sound.deathSoundName = soundJson["deathSoundName"];
      } else if (soundJson.contains("deathSound") && soundJson["deathSound"].is_string()) {
        characterTemplate.sound.deathSoundName = soundJson["deathSound"];
      }
      if (soundJson.contains("weaponSoundName") && soundJson["weaponSoundName"].is_string()) {
        characterTemplate.sound.weaponSoundName = soundJson["weaponSoundName"];
      } else if (soundJson.contains("weaponSound") && soundJson["weaponSound"].is_string()) {
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
      throw std::runtime_error("Character template already exists: " + characterTemplate.name);
    }

    characterTemplates[characterTemplate.name] = characterTemplate;
  }
}

} // namespace db
