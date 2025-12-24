#include "LoadSpecialEvents.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/json.hpp"
#include "model/SpecialEvents.h"
#include <algorithm>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace db {

static model::GameEventType getGameEventTypeFromString(const std::string& typeStr) {
  if (typeStr == "MODAL") {
    return model::GameEventType::MODAL;
  } else if (typeStr == "TALK") {
    return model::GameEventType::TALK;
  } else {
    throw std::runtime_error("Invalid eventType: " + typeStr);
  }
}

static model::GameEventChildType getGameEventChildTypeFromString(const std::string& typeStr) {
  if (typeStr == "EXEC") {
    return model::GameEventChildType::EXEC;
  } else if (typeStr == "CHOICE") {
    return model::GameEventChildType::CHOICE;
  } else if (typeStr == "SWITCH") {
    return model::GameEventChildType::SWITCH;
  } else if (typeStr == "END") {
    return model::GameEventChildType::END;
  } else {
    throw std::runtime_error("Invalid eventChildType: " + typeStr);
  }
}

static std::vector<std::string> splitParagraphs(const std::string& p) {
  if (p.empty()) {
    return {};
  }
  std::vector<std::string> paragraphs;
  std::istringstream iss(p);
  std::string line;
  while (std::getline(iss, line)) {
    if (!line.empty() || !paragraphs.empty()) {
      paragraphs.push_back(line);
    }
  }
  return paragraphs.empty() ? std::vector<std::string>{""} : paragraphs;
}

static model::AudioInfo parseAudioInfo(const nlohmann::json& audioJson) {
  model::AudioInfo audioInfo;
  if (audioJson.contains("audioName") && audioJson["audioName"].is_string()) {
    audioInfo.audioName = audioJson["audioName"];
  }
  if (audioJson.contains("volume") && audioJson["volume"].is_number_integer()) {
    audioInfo.volume = audioJson["volume"];
  }
  if (audioJson.contains("offset") && audioJson["offset"].is_number_integer()) {
    audioInfo.offset = audioJson["offset"];
  }
  return audioInfo;
}

static std::optional<model::GameEventChild> parseGameEventChild(const nlohmann::json& childJson) {
  if (!childJson.contains("eventChildType") || !childJson["eventChildType"].is_string()) {
    throw std::runtime_error("Child missing required field: eventChildType");
  }
  if (!childJson.contains("id") || !childJson["id"].is_string()) {
    throw std::runtime_error("Child missing required field: id");
  }

  std::string eventChildTypeStr = childJson["eventChildType"];
  
  // Skip COMMENT nodes
  if (eventChildTypeStr == "COMMENT") {
    return std::nullopt;
  }

  model::GameEventChildType childType = getGameEventChildTypeFromString(eventChildTypeStr);

  if (childType == model::GameEventChildType::EXEC) {
    model::GameEventChildExec execNode;
    execNode.eventChildType = model::GameEventChildType::EXEC;
    execNode.id = childJson["id"];
    
    if (childJson.contains("p") && childJson["p"].is_string()) {
      execNode.paragraphs = splitParagraphs(childJson["p"]);
    } else {
      execNode.paragraphs = {};
    }
    
    if (childJson.contains("execStr") && childJson["execStr"].is_string()) {
      execNode.execStr = childJson["execStr"];
    } else {
      execNode.execStr = "";
    }
    
    if (childJson.contains("next") && childJson["next"].is_string()) {
      execNode.next = childJson["next"];
    } else {
      execNode.next = "";
    }
    
    if (childJson.contains("autoAdvance") && childJson["autoAdvance"].is_boolean()) {
      execNode.autoAdvance = childJson["autoAdvance"];
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
    choiceNode.id = childJson["id"];
    
    if (childJson.contains("text") && childJson["text"].is_string()) {
      choiceNode.text = childJson["text"];
    } else {
      choiceNode.text = "";
    }
    
    if (childJson.contains("choices") && childJson["choices"].is_array()) {
      for (const auto& choiceJson : childJson["choices"]) {
        model::Choice choice;
        if (choiceJson.contains("text") && choiceJson["text"].is_string()) {
          choice.text = choiceJson["text"];
        }
        if (choiceJson.contains("prefixText") && choiceJson["prefixText"].is_string()) {
          choice.prefixText = choiceJson["prefixText"];
        }
        if (choiceJson.contains("conditionStr") && choiceJson["conditionStr"].is_string()) {
          choice.conditionStr = choiceJson["conditionStr"];
        }
        if (choiceJson.contains("evalStr") && choiceJson["evalStr"].is_string()) {
          choice.evalStr = choiceJson["evalStr"];
        }
        if (choiceJson.contains("next") && choiceJson["next"].is_string()) {
          choice.next = choiceJson["next"];
        }
        choiceNode.choices.push_back(choice);
      }
    }
    
    if (childJson.contains("audioInfo") && childJson["audioInfo"].is_object()) {
      choiceNode.audioInfo = parseAudioInfo(childJson["audioInfo"]);
    }
    
    return choiceNode;
  } else if (childType == model::GameEventChildType::SWITCH) {
    model::GameEventChildSwitch switchNode;
    switchNode.eventChildType = model::GameEventChildType::SWITCH;
    switchNode.id = childJson["id"];
    
    if (childJson.contains("defaultNext") && childJson["defaultNext"].is_string()) {
      switchNode.defaultNext = childJson["defaultNext"];
    } else {
      switchNode.defaultNext = "";
    }
    
    if (childJson.contains("cases") && childJson["cases"].is_array()) {
      for (const auto& caseJson : childJson["cases"]) {
        model::SwitchCase switchCase;
        if (caseJson.contains("conditionStr") && caseJson["conditionStr"].is_string()) {
          switchCase.conditionStr = caseJson["conditionStr"];
        }
        if (caseJson.contains("next") && caseJson["next"].is_string()) {
          switchCase.next = caseJson["next"];
        }
        switchNode.cases.push_back(switchCase);
      }
    }
    
    return switchNode;
  } else if (childType == model::GameEventChildType::END) {
    model::GameEventChildEnd endNode;
    endNode.eventChildType = model::GameEventChildType::END;
    endNode.id = childJson["id"];
    
    if (childJson.contains("next") && childJson["next"].is_string()) {
      endNode.next = childJson["next"];
    } else {
      endNode.next = "";
    }
    
    return endNode;
  } else {
    throw std::runtime_error("Unsupported eventChildType: " + eventChildTypeStr);
  }
}

void loadSpecialEvents(const std::string& specialEventsFilePath,
                       std::unordered_map<std::string, model::GameEvent>& specialEvents) {
  std::vector<std::string> emptyEventsToLoad;
  loadSpecialEvents(specialEventsFilePath, specialEvents, emptyEventsToLoad);
}

void loadSpecialEvents(const std::string& specialEventsFilePath,
                       std::unordered_map<std::string, model::GameEvent>& specialEvents,
                       std::vector<std::string>& eventsToLoad) {
  std::string fileContent = sdl2w::loadFileAsString(specialEventsFilePath);
  
  nlohmann::json jsonData;
  try {
    // Parse with comments enabled (ignore_comments = true)
    jsonData = nlohmann::json::parse(fileContent, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Failed to parse JSON file " + specialEventsFilePath + ": " + e.what());
  }

  if (!jsonData.is_array()) {
    throw std::runtime_error("JSON file must contain an array of events");
  }

  bool filterByEventsToLoad = !eventsToLoad.empty();
  
  for (const auto& eventJson : jsonData) {
    model::GameEvent gameEvent;

    // Required fields
    if (!eventJson.contains("id") || !eventJson["id"].is_string()) {
      throw std::runtime_error("Event missing required field: id");
    }
    gameEvent.id = eventJson["id"];
    
    // Filter by eventsToLoad if specified
    if (filterByEventsToLoad) {
      if (std::find(eventsToLoad.begin(), eventsToLoad.end(), gameEvent.id) == eventsToLoad.end()) {
        continue;
      }
    }

    if (!eventJson.contains("title") || !eventJson["title"].is_string()) {
      throw std::runtime_error("Event missing required field: title");
    }
    gameEvent.title = eventJson["title"];

    if (!eventJson.contains("eventType") || !eventJson["eventType"].is_string()) {
      throw std::runtime_error("Event missing required field: eventType");
    }
    std::string eventTypeStr = eventJson["eventType"];
    gameEvent.eventType = getGameEventTypeFromString(eventTypeStr);

    if (!eventJson.contains("icon") || !eventJson["icon"].is_string()) {
      throw std::runtime_error("Event missing required field: icon");
    }
    gameEvent.icon = eventJson["icon"];

    // Parse vars
    if (eventJson.contains("vars") && eventJson["vars"].is_array()) {
      for (const auto& varJson : eventJson["vars"]) {
        model::Variable var;
        if (varJson.contains("id") && varJson["id"].is_string()) {
          var.id = varJson["id"];
        }
        if (varJson.contains("key") && varJson["key"].is_string()) {
          var.key = varJson["key"];
        }
        if (varJson.contains("value") && varJson["value"].is_string()) {
          var.value = varJson["value"];
        }
        if (varJson.contains("importFrom") && varJson["importFrom"].is_string()) {
          var.importFrom = varJson["importFrom"];
        }
        gameEvent.vars.push_back(var);
      }
    }

    // Parse children
    if (eventJson.contains("children") && eventJson["children"].is_array()) {
      for (const auto& childJson : eventJson["children"]) {
        try {
          auto childOpt = parseGameEventChild(childJson);
          if (childOpt.has_value()) {
            gameEvent.children.push_back(childOpt.value());
          }
          // Skip COMMENT nodes (they return nullopt)
        } catch (const std::exception& e) {
          // Skip nodes that fail to parse
          continue;
        }
      }
    }

    // Check for duplicate IDs
    if (specialEvents.find(gameEvent.id) != specialEvents.end()) {
      throw std::runtime_error("Event already exists: " + gameEvent.id);
    }

    specialEvents[gameEvent.id] = gameEvent;
  }
}

} // namespace db

