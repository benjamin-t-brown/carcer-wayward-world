#include "LoadCharacterTemplates.h"
#include "bmin/StringInterop.h"
#include "lib/Json.h"
#include "sdl2w/AssetLoader.h"
#include "model/stats/CharacterStats.h"
#include <stdexcept>

namespace {

model::CharacterTemplateType
getCharacterTemplateTypeFromString(const bmin::String& typeStr) {
  if (typeStr == "TOWNSPERSON") {
    return model::CharacterTemplateType::TOWNSPERSON;
  } else if (typeStr == "TOWNSPERSON_STATIC") {
    return model::CharacterTemplateType::TOWNSPERSON_STATIC;
  } else if (typeStr == "ENEMY") {
    return model::CharacterTemplateType::ENEMY;
  } else if (typeStr == "ENEMY_STATIC") {
    return model::CharacterTemplateType::ENEMY_STATIC;
  }
  throw std::runtime_error((bmin::String("Invalid character template type: ") + typeStr).cStr());
}

model::CharacterTemplateBehaviorName
getCharacterTemplateBehaviorNameFromString(const bmin::String& behaviorStr) {
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
  throw std::runtime_error((bmin::String("Invalid character template behavior: ") + behaviorStr)
                               .cStr());
}

bmin::String getStringFromCharacterTemplateBehaviorName(
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

void loadIntField(const Json& json, const char* fieldName, int& out) {
  if (json.contains(fieldName)) {
    out = json[fieldName].get<int>();
  }
}

void loadGenericCombatStats(const Json& json, model::GenericCombatStats& stats) {
  loadIntField(json, "str", stats.str);
  loadIntField(json, "mnd", stats.mnd);
  loadIntField(json, "con", stats.con);
  loadIntField(json, "agi", stats.agi);
  loadIntField(json, "lck", stats.lck);
}

void loadWeaponMasteryStats(const Json& json, model::WeaponMasteryStats& stats) {
  loadIntField(json, "edged", stats.edged);
  loadIntField(json, "pole", stats.pole);
  loadIntField(json, "blunt", stats.blunt);
  loadIntField(json, "range", stats.range);
  loadIntField(json, "unarmed", stats.unarmed);
}

void loadMagicMasteryStats(const Json& json, model::MagicMasteryStats& stats) {
  loadIntField(json, "mana", stats.mana);
  loadIntField(json, "abilityPower", stats.abilityPower);
  loadIntField(json, "attunement", stats.attunement);
  loadIntField(json, "faith", stats.faith);
  loadIntField(json, "lore", stats.lore);
}

void loadBodyMasteryStats(const Json& json, model::BodyMasteryStats& stats) {
  loadIntField(json, "resistPhysical", stats.resistPhysical);
  loadIntField(json, "resistMagical", stats.resistMagical);
  loadIntField(json, "healingEffectiveness", stats.healingEffectiveness);
  loadIntField(json, "dr", stats.dr);
  loadIntField(json, "armorTraining", stats.armorTraining);
}

void loadTrainableCombatStats(const Json& json, model::TrainableCombatStats& stats) {
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

void loadCharacterSkills(const Json& json, model::CharacterSkills& skills) {
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

void loadCharacterStats(const Json& characterJson, model::CharacterStats& stats) {
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

bmin::String getSpriteOffsetAsString(const Json& characterJson) {
  if (!characterJson.contains("spriteOffset")) {
    throw std::runtime_error("Character missing required field: spriteOffset");
  }
  if (characterJson["spriteOffset"].is_number_integer()) {
    return bmin::toString(characterJson["spriteOffset"].get<int>());
  }
  if (characterJson["spriteOffset"].is_string()) {
    return characterJson["spriteOffset"].get<bmin::String>();
  }
  throw std::runtime_error("Character spriteOffset must be a number or string");
}

} // namespace

namespace db {

void loadCharacterTemplates(
    const bmin::String& charactersFilePath,
    bmin::Map<bmin::String, model::CharacterTemplate>& characterTemplates) {
  const bmin::String fileContent = sdl2w::loadFileAsString(bmin::toStringView(charactersFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((bmin::String("Failed to parse JSON file ") +
                              charactersFilePath.cStr() + ": " + e.what())
                                 .cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of characters");
  }

  for (const auto& characterJson : jsonData) {
    model::CharacterTemplate characterTemplate;

    if (!characterJson.contains("type") || !characterJson["type"].is_string()) {
      throw std::runtime_error("Character missing required field: type");
    }
    characterTemplate.type = getCharacterTemplateTypeFromString(characterJson["type"].get<bmin::String>());

    if (!characterJson.contains("name") || !characterJson["name"].is_string()) {
      throw std::runtime_error("Character missing required field: name");
    }
    characterTemplate.name = characterJson["name"].get<bmin::String>();

    if (!characterJson.contains("label") || !characterJson["label"].is_string()) {
      throw std::runtime_error("Character missing required field: label");
    }
    characterTemplate.label = characterJson["label"].get<bmin::String>();

    if (!characterJson.contains("spritesheet") ||
        !characterJson["spritesheet"].is_string()) {
      throw std::runtime_error("Character missing required field: spritesheet");
    }
    characterTemplate.spritesheetName = characterJson["spritesheet"].get<bmin::String>();
    characterTemplate.spriteOffset = getSpriteOffsetAsString(characterJson);

    if (characterJson.contains("talk") && characterJson["talk"].is_object()) {
      const auto& talkJson = characterJson["talk"];
      if (talkJson.contains("talkName") && talkJson["talkName"].is_string()) {
        characterTemplate.talk.talkName = talkJson["talkName"].get<bmin::String>();
      }
      if (talkJson.contains("portraitName") && talkJson["portraitName"].is_string()) {
        characterTemplate.talk.portraitName = talkJson["portraitName"].get<bmin::String>();
      }
    }

    if (characterJson.contains("behavior") && characterJson["behavior"].is_object()) {
      const auto& behaviorJson = characterJson["behavior"];
      if (behaviorJson.contains("behaviorName") &&
          behaviorJson["behaviorName"].is_string()) {
        const bmin::String behaviorStr = behaviorJson["behaviorName"].get<bmin::String>();
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
        characterTemplate.combat.hp = combatJson["hp"].get<int>();
      }
      if (combatJson.contains("mp")) {
        characterTemplate.combat.mp = combatJson["mp"].get<int>();
      }
      if (combatJson.contains("dropTable") && combatJson["dropTable"].is_string()) {
        characterTemplate.combat.dropTable = combatJson["dropTable"].get<bmin::String>();
      }
    }

    if (characterJson.contains("sound") && characterJson["sound"].is_object()) {
      const auto& soundJson = characterJson["sound"];
      if (soundJson.contains("deathSoundName") &&
          soundJson["deathSoundName"].is_string()) {
        characterTemplate.sound.deathSoundName = soundJson["deathSoundName"].get<bmin::String>();
      } else if (soundJson.contains("deathSound") &&
                 soundJson["deathSound"].is_string()) {
        characterTemplate.sound.deathSoundName = soundJson["deathSound"].get<bmin::String>();
      }
      if (soundJson.contains("weaponSoundName") &&
          soundJson["weaponSoundName"].is_string()) {
        characterTemplate.sound.weaponSoundName = soundJson["weaponSoundName"].get<bmin::String>();
      } else if (soundJson.contains("weaponSound") &&
                 soundJson["weaponSound"].is_string()) {
        characterTemplate.sound.weaponSoundName = soundJson["weaponSound"].get<bmin::String>();
      }
    }

    if (characterJson.contains("statuses") && characterJson["statuses"].is_array()) {
      for (const auto& statusJson : characterJson["statuses"]) {
        if (statusJson.contains("status") && statusJson["status"].is_string()) {
          characterTemplate.statuses.pushBack({statusJson["status"].get<bmin::String>()});
        }
      }
    }

    if (characterJson.contains("vision") && characterJson["vision"].is_object()) {
      const auto& visionJson = characterJson["vision"];
      if (visionJson.contains("radius")) {
        characterTemplate.vision.radius = visionJson["radius"].get<int>();
      }
    }

    if (characterTemplates.contains(characterTemplate.name)) {
      throw std::runtime_error((bmin::String("Character template already exists: ") +
                                characterTemplate.name)
                                   .cStr());
    }

    characterTemplates[characterTemplate.name] = characterTemplate;
  }
}

} // namespace db
