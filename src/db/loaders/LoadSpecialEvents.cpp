#include "LoadSpecialEvents.h"
#include "lib/Json.h"
#include "lib/StringUtil.h"
#include "lib/sdl2w/AssetLoader.h"
#include "model/templates/SpecialEvents.h"
#include <algorithm>
#include <optional>
#include <stdexcept>

namespace db {

static model::GameEventType getGameEventTypeFromString(const String& typeStr) {
  if (typeStr == "MODAL") {
    return model::GameEventType::MODAL;
  } else if (typeStr == "TALK") {
    return model::GameEventType::TALK;
  } else {
    throw std::runtime_error((String("Invalid eventType: ") + typeStr).cStr());
  }
}

static model::GameEventChildType getGameEventChildTypeFromString(const String& typeStr) {
  if (typeStr == "EXEC") {
    return model::GameEventChildType::EXEC;
  } else if (typeStr == "CHOICE") {
    return model::GameEventChildType::CHOICE;
  } else if (typeStr == "SWITCH") {
    return model::GameEventChildType::SWITCH;
  } else if (typeStr == "END") {
    return model::GameEventChildType::END;
  } else {
    throw std::runtime_error((String("Invalid eventChildType: ") + typeStr).cStr());
  }
}

static bmin::DynArray<String> splitParagraphs(const String& p) {
  if (p.empty()) {
    return {};
  }
  const bmin::DynArray<String> lines = strutil::splitLines(p);
  bmin::DynArray<String> paragraphs;
  paragraphs.reserve(lines.size());
  for (size_t i = 0; i < lines.size(); ++i) {
    if (!lines[i].empty() || !paragraphs.empty()) {
      paragraphs.pushBack(lines[i]);
    }
  }
  if (paragraphs.empty()) {
    paragraphs.pushBack("");
  }
  return paragraphs;
}

static model::AudioInfo parseAudioInfo(const Json& audioJson) {
  model::AudioInfo audioInfo;
  if (audioJson.contains("audioName") && audioJson["audioName"].is_string()) {
    audioInfo.audioName = audioJson["audioName"].get<String>();
  }
  if (audioJson.contains("volume") && audioJson["volume"].is_number_integer()) {
    audioInfo.volume = audioJson["volume"].get<int>();
  }
  if (audioJson.contains("offset") && audioJson["offset"].is_number_integer()) {
    audioInfo.offset = audioJson["offset"].get<int>();
  }
  return audioInfo;
}

static std::optional<model::GameEventChild> parseGameEventChild(const Json& childJson) {
  if (!childJson.contains("eventChildType") || !childJson["eventChildType"].is_string()) {
    throw std::runtime_error("Child missing required field: eventChildType");
  }
  if (!childJson.contains("id") || !childJson["id"].is_string()) {
    throw std::runtime_error("Child missing required field: id");
  }

  const String eventChildTypeStr = childJson["eventChildType"].get<String>();

  if (eventChildTypeStr == "COMMENT") {
    return std::nullopt;
  }

  model::GameEventChildType childType = getGameEventChildTypeFromString(eventChildTypeStr);

  if (childType == model::GameEventChildType::EXEC) {
    model::GameEventChildExec execNode;
    execNode.eventChildType = model::GameEventChildType::EXEC;
    execNode.id = childJson["id"].get<String>();

    if (childJson.contains("p") && childJson["p"].is_string()) {
      execNode.paragraphs = splitParagraphs(childJson["p"].get<String>());
    } else {
      execNode.paragraphs = {};
    }

    if (childJson.contains("execStr") && childJson["execStr"].is_string()) {
      execNode.execStr = childJson["execStr"].get<String>();
    } else {
      execNode.execStr = "";
    }

    if (childJson.contains("next") && childJson["next"].is_string()) {
      execNode.next = childJson["next"].get<String>();
    } else {
      execNode.next = "";
    }

    if (childJson.contains("autoAdvance") && childJson["autoAdvance"].is_boolean()) {
      execNode.autoAdvance = childJson["autoAdvance"].get<bool>();
    } else {
      execNode.autoAdvance = false;
    }

    if (childJson.contains("audioInfo") && childJson["audioInfo"].is_object()) {
      execNode.audioInfo = parseAudioInfo(childJson["audioInfo"]);
    }

    return execNode;
  } else if (childType == model::GameEventChildType::CHOICE) {
    model::GameEventChildChoice choiceNode;
    choiceNode.eventChildType = model::GameEventChildType::CHOICE;
    choiceNode.id = childJson["id"].get<String>();

    if (childJson.contains("text") && childJson["text"].is_string()) {
      choiceNode.text = childJson["text"].get<String>();
    } else {
      choiceNode.text = "";
    }

    if (childJson.contains("choices") && childJson["choices"].is_array()) {
      for (const auto& choiceJson : childJson["choices"]) {
        model::Choice choice;
        if (choiceJson.contains("text") && choiceJson["text"].is_string()) {
          choice.text = choiceJson["text"].get<String>();
        }
        if (choiceJson.contains("prefixText") && choiceJson["prefixText"].is_string()) {
          choice.prefixText = choiceJson["prefixText"].get<String>();
        }
        if (choiceJson.contains("conditionStr") && choiceJson["conditionStr"].is_string()) {
          choice.conditionStr = choiceJson["conditionStr"].get<String>();
        }
        if (choiceJson.contains("evalStr") && choiceJson["evalStr"].is_string()) {
          choice.evalStr = choiceJson["evalStr"].get<String>();
        }
        if (choiceJson.contains("next") && choiceJson["next"].is_string()) {
          choice.next = choiceJson["next"].get<String>();
        }
        if (choiceJson.contains("switchText") && choiceJson["switchText"].is_array()) {
          for (const auto& switchTextJson : choiceJson["switchText"]) {
            model::ChoiceSwitchText switchText;
            if (switchTextJson.contains("conditionStr") &&
                switchTextJson["conditionStr"].is_string()) {
              switchText.conditionStr = switchTextJson["conditionStr"].get<String>();
            }
            if (switchTextJson.contains("text") && switchTextJson["text"].is_string()) {
              switchText.text = switchTextJson["text"].get<String>();
            }
            choice.switchText.pushBack(switchText);
          }
        }
        choiceNode.choices.pushBack(choice);
      }
    }

    if (childJson.contains("audioInfo") && childJson["audioInfo"].is_object()) {
      choiceNode.audioInfo = parseAudioInfo(childJson["audioInfo"]);
    }

    return choiceNode;
  } else if (childType == model::GameEventChildType::SWITCH) {
    model::GameEventChildSwitch switchNode;
    switchNode.eventChildType = model::GameEventChildType::SWITCH;
    switchNode.id = childJson["id"].get<String>();

    if (childJson.contains("defaultNext") && childJson["defaultNext"].is_string()) {
      switchNode.defaultNext = childJson["defaultNext"].get<String>();
    } else {
      switchNode.defaultNext = "";
    }

    if (childJson.contains("cases") && childJson["cases"].is_array()) {
      for (const auto& caseJson : childJson["cases"]) {
        model::SwitchCase switchCase;
        if (caseJson.contains("conditionStr") && caseJson["conditionStr"].is_string()) {
          switchCase.conditionStr = caseJson["conditionStr"].get<String>();
        }
        if (caseJson.contains("next") && caseJson["next"].is_string()) {
          switchCase.next = caseJson["next"].get<String>();
        }
        switchNode.cases.pushBack(switchCase);
      }
    }

    return switchNode;
  } else if (childType == model::GameEventChildType::END) {
    model::GameEventChildEnd endNode;
    endNode.eventChildType = model::GameEventChildType::END;
    endNode.id = childJson["id"].get<String>();

    if (childJson.contains("next") && childJson["next"].is_string()) {
      endNode.next = childJson["next"].get<String>();
    } else {
      endNode.next = "";
    }

    return endNode;
  } else {
    throw std::runtime_error((String("Unsupported eventChildType: ") + eventChildTypeStr)
                                 .cStr());
  }
}

void loadSpecialEvents(const String& specialEventsFilePath,
                       bmin::Map<String, model::GameEvent>& specialEvents) {
  bmin::DynArray<String> emptyEventsToLoad;
  loadSpecialEvents(specialEventsFilePath, specialEvents, emptyEventsToLoad);
}

void loadSpecialEvents(const String& specialEventsFilePath,
                       bmin::Map<String, model::GameEvent>& specialEvents,
                       bmin::DynArray<String>& eventsToLoad) {
  const String fileContent = sdl2w::loadFileAsString(bmin::toStringView(specialEventsFilePath));

  Json jsonData;
  try {
    jsonData = Json::parse(fileContent.cStr(), nullptr, true, true);
  } catch (const Json::parse_error& e) {
    throw std::runtime_error((String("Failed to parse JSON file ") + specialEventsFilePath.cStr() +
                              ": " + e.what())
                                 .cStr());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of events");
  }

  const bool filterByEventsToLoad = !eventsToLoad.empty();

  for (const auto& eventJson : jsonData) {
    model::GameEvent gameEvent;

    if (!eventJson.contains("id") || !eventJson["id"].is_string()) {
      throw std::runtime_error("Event missing required field: id");
    }
    gameEvent.id = eventJson["id"].get<String>();

    if (filterByEventsToLoad) {
      if (std::find(eventsToLoad.begin(), eventsToLoad.end(), gameEvent.id) ==
          eventsToLoad.end()) {
        continue;
      }
    }

    if (!eventJson.contains("title") || !eventJson["title"].is_string()) {
      throw std::runtime_error("Event missing required field: title");
    }
    gameEvent.title = eventJson["title"].get<String>();

    if (!eventJson.contains("eventType") || !eventJson["eventType"].is_string()) {
      throw std::runtime_error("Event missing required field: eventType");
    }
    const String eventTypeStr = eventJson["eventType"].get<String>();
    gameEvent.eventType = getGameEventTypeFromString(eventTypeStr);

    if (!eventJson.contains("icon") || !eventJson["icon"].is_string()) {
      throw std::runtime_error("Event missing required field: icon");
    }
    gameEvent.icon = eventJson["icon"].get<String>();

    if (eventJson.contains("vars") && eventJson["vars"].is_array()) {
      for (const auto& varJson : eventJson["vars"]) {
        model::Variable var;
        if (varJson.contains("id") && varJson["id"].is_string()) {
          var.id = varJson["id"].get<String>();
        }
        if (varJson.contains("key") && varJson["key"].is_string()) {
          var.key = varJson["key"].get<String>();
        }
        if (varJson.contains("value") && varJson["value"].is_string()) {
          var.value = varJson["value"].get<String>();
        }
        if (varJson.contains("importFrom") && varJson["importFrom"].is_string()) {
          var.importFrom = varJson["importFrom"].get<String>();
        }
        gameEvent.vars.pushBack(var);
      }
    }

    if (eventJson.contains("children") && eventJson["children"].is_array()) {
      for (const auto& childJson : eventJson["children"]) {
        try {
          auto childOpt = parseGameEventChild(childJson);
          if (childOpt.has_value()) {
            gameEvent.children.pushBack(childOpt.value());
          }
        } catch (const std::exception&) {
          continue;
        }
      }
    }

    if (specialEvents.contains(gameEvent.id)) {
      throw std::runtime_error((String("Event already exists: ") + gameEvent.id).cStr());
    }

    specialEvents[gameEvent.id] = gameEvent;
  }
}

} // namespace db
