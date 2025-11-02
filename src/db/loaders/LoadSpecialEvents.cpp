#include "LoadSpecialEvents.h"
#include "lib/sdl2w/AssetLoader.h"
#include "lib/sdl2w/Logger.h"
#include <stdexcept>

namespace db {

static model::GameEventType parseEventType(const std::string& typeStr) {
  auto trimmed = sdl2w::trim(typeStr);
  if (trimmed == "modal") {
    return model::GameEventType::MODAL;
  } else if (trimmed == "talk") {
    return model::GameEventType::TALK;
  }
  throw std::runtime_error("Unknown event type: " + typeStr);
}

static model::GameEventChildType parseChildType(const std::string& typeStr) {
  auto trimmed = sdl2w::trim(typeStr);
  if (trimmed == "keyword") {
    return model::GameEventChildType::KEYWORD;
  } else if (trimmed == "choice") {
    return model::GameEventChildType::CHOICE;
  } else if (trimmed == "end") {
    return model::GameEventChildType::END;
  } else if (trimmed == "exec") {
    return model::GameEventChildType::EXEC;
  } else if (trimmed == "switch") {
    return model::GameEventChildType::SWITCH;
  }
  throw std::runtime_error("Unknown child type: " + typeStr);
}

// static bool isCommentOrEmpty(const std::string& line) {
//   auto trimmed = sdl2w::trim(line);
//   if (trimmed.empty()) {
//     return true;
//   }
//   // Check if line is a comment (// at start or after whitespace)
//   size_t commentPos = trimmed.find("//");
//   if (commentPos == 0) {
//     return true;
//   }
//   return false;
// }

// static std::string extractQuotedString(const std::string& str) {
//   size_t firstQuote = str.find('"');
//   if (firstQuote == std::string::npos) {
//     return sdl2w::trim(str);
//   }
//   size_t secondQuote = str.find('"', firstQuote + 1);
//   if (secondQuote == std::string::npos) {
//     return str.substr(firstQuote + 1);
//   }
//   return str.substr(firstQuote + 1, secondQuote - firstQuote - 1);
// }

// static bool isIndented(const std::string& line) {
//   if (line.empty()) {
//     return false;
//   }
//   return line[0] == ' ' || line[0] == '\t';
// }

void loadSpecialEvents(const std::string& eventsFilePath,
                       std::unordered_map<std::string, model::GameEvent>& gameEvents) {
  std::string fileContent = sdl2w::loadFileAsString(eventsFilePath);
  std::vector<std::string> lines;
  sdl2w::split(fileContent, "\n", lines);

  model::GameEvent currentEvent;
  std::optional<model::GameEventChild> currentChild;
  bool inEvent = false;
  // bool inMultiLineProperty = false;
  std::string multiLinePropertyName;
  std::string multiLinePropertyValue;
  model::GameEventChildType currentChildType;

  for (size_t i = 0; i < lines.size(); ++i) {
    const std::string& line = sdl2w::trim(lines[i]);
    // auto trimmed = sdl2w::trim(line);

    // Skip comments and empty lines (but check for multi-line first)
    // if (!inMultiLineProperty && isCommentOrEmpty(line)) {
    //   continue;
    // }

    // // Handle multi-line properties (continuation lines)
    // if (inMultiLineProperty) {
    //   auto trimmedLine = sdl2w::trim(line);
    //   if (trimmedLine[0] == '|') {
    //     // This is a continuation of the multi-line property
    //     multiLinePropertyValue += line;
    //     continue;
    //   } else {
    //     // End of multi-line property, process it
    //     inMultiLineProperty = false;
    //     // Process the accumulated multi-line value
    //     // This will be handled below when we process the property
    //   }
    // }

    // Parse event definition: #eventId,title,icon or #eventId,title,icon,eventType
    if (line[0] == '!') {
      // Save previous event if exists
      if (inEvent) {
        if (currentChild.has_value()) {
          currentEvent.children.push_back(currentChild.value());
        }
        gameEvents[currentEvent.id] = currentEvent;
      }

      // Start new event
      inEvent = true;
      currentEvent = model::GameEvent();
      currentChild.reset();
      currentEvent.eventType = model::GameEventType::MODAL; // default

      std::vector<std::string> parts;
      sdl2w::split(line.substr(1), ",", parts);
      if (parts.size() < 4) {
        throw std::runtime_error("Invalid event definition: " + line);
      }
      currentEvent.id = sdl2w::trim(parts[0]);
      currentEvent.eventType = parseEventType(parts[1]);
      currentEvent.title = sdl2w::trim(parts[2]);
      currentEvent.icon = sdl2w::trim(parts[3]);
      continue;
    }

    if (!inEvent) {
      continue; // Skip lines outside of events
    }

    // Parse variable definition: @varName=value
    if (line[0] == '@') {
      size_t eqPos = line.find('=');
      if (eqPos == std::string::npos) {
        throw std::runtime_error("Invalid variable definition: " + line);
      }
      std::string varName = sdl2w::trim(line.substr(1, eqPos - 1));
      std::string varValue = sdl2w::trim(line.substr(eqPos + 1));
      model::VariableValue varVal;
      varVal.str = varValue;
      currentEvent.vars[varName] = varVal;
      continue;
    }

    // Parse child definition: >childId,childType
    if (line[0] == '>') {
      // Save previous child if exists
      if (currentChild.has_value()) {
        currentEvent.children.push_back(currentChild.value());
      }

      std::vector<std::string> parts;
      sdl2w::split(line.substr(1), ",", parts);
      if (parts.size() < 2) {
        throw std::runtime_error("Invalid child definition: " + line);
      }
      std::string childId = sdl2w::trim(parts[0]);
      currentChildType = parseChildType(parts[1]);

      // Create appropriate child type
      switch (currentChildType) {
      case model::GameEventChildType::KEYWORD: {
        model::GameEventChildKeyword child;
        child.id = childId;
        currentChild = child;
        break;
      }
      case model::GameEventChildType::CHOICE: {
        model::GameEventChildChoice child;
        child.id = childId;
        currentChild = child;
        break;
      }
      case model::GameEventChildType::SWITCH: {
        model::GameEventChildSwitch child;
        child.id = childId;
        currentChild = child;
        break;
      }
      case model::GameEventChildType::EXEC: {
        model::GameEventChildExec child;
        child.id = childId;
        currentChild = child;
        break;
      }
      case model::GameEventChildType::END: {
        model::GameEventChildEnd child;
        child.id = childId;
        currentChild = child;
        break;
      }
      }
      continue;
    }

    // Parse property: +propName: value
    if (line[0] == '+') {
      size_t colonPos = line.find(':');
      if (colonPos == std::string::npos) {
        throw std::runtime_error("Invalid property definition: " + line);
      }

      std::string propName = sdl2w::trim(line.substr(1, colonPos - 1));
      std::string propValue = line.substr(colonPos + 1);

      LOG(INFO) << "Property: " << propName << ", Value: " << propValue << LOG_ENDL;

      // // Check if this might be a multi-line property (kSwitch or others that continue)
      // bool isMultiLineStart = false;
      // if (propName == "kSwitch") {
      //   // Check if next line is continuation
      //   if (i + 1 < lines.size()) {
      //     const std::string& nextLine = lines[i + 1];
      //     if (isIndented(nextLine) || (nextLine[0] == '|')) {
      //       isMultiLineStart = true;
      //       inMultiLineProperty = true;
      //       multiLinePropertyName = propName;
      //       multiLinePropertyValue = sdl2w::trim(propValue);
      //     }
      //   }
      // }

      // if (isMultiLineStart) {
      //   continue; // Will be processed when multi-line ends
      // }

      propValue = sdl2w::trim(propValue);

      // Process property based on current child type
      if (!currentChild.has_value()) {
        throw std::runtime_error("Property found without child: " + line);
      }

      // Handle keyword child properties
      if (std::holds_alternative<model::GameEventChildKeyword>(currentChild.value())) {
        auto& child = std::get<model::GameEventChildKeyword>(currentChild.value());

        if (propName == "k") {
          std::vector<std::string> parts;
          sdl2w::split(propValue, "|", parts);
          if (parts.size() < 2) {
            throw std::runtime_error("Invalid k property: " + line);
          }
          std::string keywordName = sdl2w::trim(parts[0]);
          std::string keywordText = sdl2w::trim(parts[1]);
          model::KeywordData keywordData;
          keywordData.keywordType = model::KeywordType::K;
          model::KeywordDataK kData;
          kData.text = keywordText;
          keywordData.k = kData;
          child.keywords[keywordName] = keywordData;
        } else if (propName == "kDup") {
          std::vector<std::string> parts;
          sdl2w::split(propValue, "|", parts);
          if (parts.size() < 2) {
            throw std::runtime_error("Invalid kDup property: " + line);
          }
          std::string keywordName = sdl2w::trim(parts[0]);
          std::string targetKeyword = sdl2w::trim(parts[1]);
          model::KeywordData keywordData;
          keywordData.keywordType = model::KeywordType::K_DUP;
          model::KeywordDataKDup kDupData;
          kDupData.keyword = targetKeyword;
          keywordData.kDup = kDupData;
          child.keywords[keywordName] = keywordData;
        } else if (propName == "kSwitch") {
          // Parse kSwitch with its cases (multi-line)
          model::KeywordData keywordData;
          keywordData.keywordType = model::KeywordType::K_SWITCH;
          model::KeywordDataKSwitch kSwitchData;

          std::string lineAggregated = sdl2w::trim(propValue);

          // Parse cases from subsequent lines
          size_t caseLineIndex = i + 1;
          while (caseLineIndex < lines.size()) {
            const std::string& caseLine = sdl2w::trim(lines[caseLineIndex]);

            // Check if this is still part of the kSwitch cases
            if (caseLine[0] != '|') {
              break; // End of cases
            }
            lineAggregated += caseLine;
            ++caseLineIndex;
          }
          i = caseLineIndex - 1; // Skip processed lines

          std::vector<std::string> parts;
          sdl2w::split(lineAggregated, "|", parts);
          if (parts.size() < 2) {
            throw std::runtime_error("Invalid kSwitch property: " + line);
          }
          std::string switchKeyword = sdl2w::trim(parts[0]);
          std::string defaultNext = sdl2w::trim(parts[1]);
          for (size_t caseI = 1; caseI < parts.size(); ++caseI) {
            const std::string& part = parts[caseI];
            std::vector<std::string> caseParts;
            sdl2w::split(part, ":", caseParts);
            if (caseParts.size() >= 2) {
              std::string conditionStr = sdl2w::trim(caseParts[0]);
              std::string nextId = sdl2w::trim(caseParts[1]);
              if (conditionStr == "default") {
                kSwitchData.defaultNext = nextId;
              } else {
                model::KeywordCheck check;
                check.conditionStr = conditionStr;
                check.next = nextId;
                kSwitchData.checks.push_back(check);
              }
            }
          }

          if (kSwitchData.defaultNext.empty()) {
            throw std::runtime_error("Default next is required for kSwitch: " + line);
          }

          keywordData.kSwitch = kSwitchData;
          child.keywords[switchKeyword] = keywordData;
        } else if (propName == "kChild") {
          std::vector<std::string> parts;
          sdl2w::split(propValue, "|", parts);
          if (parts.size() < 2) {
            throw std::runtime_error("Invalid kChild property: " + line);
          }
          std::string keywordName = sdl2w::trim(parts[0]);
          std::string nextId = sdl2w::trim(parts[1]);
          model::KeywordData keywordData;
          keywordData.keywordType = model::KeywordType::K_CHILD;
          model::KeywordDataKChild kChildData;
          kChildData.next = nextId;
          keywordData.kChild = kChildData;
          child.keywords[keywordName] = keywordData;
        }
        currentChild = child;
      }
      // Handle choice child properties
      if (std::holds_alternative<model::GameEventChildChoice>(currentChild.value())) {
        auto& child = std::get<model::GameEventChildChoice>(currentChild.value());

        if (propName == "p") {
          child.text = sdl2w::trim(propValue);
        } else if (propName == "c") {
          std::vector<std::string> parts;
          sdl2w::split(propValue, "|", parts);
          if (parts.size() < 2) {
            throw std::runtime_error("Invalid c property: " + line);
          }
          model::Choice choice;
          choice.next = sdl2w::trim(parts[0]);
          choice.text = sdl2w::trim(parts[1]);
          if (parts.size() >= 3) {
            choice.conditionStr = sdl2w::trim(parts[2]);
          }
          child.choices.push_back(choice);
        }
        currentChild = child;
      }
      // Handle switch child properties
      else if (std::holds_alternative<model::GameEventChildSwitch>(
                   currentChild.value())) {
        auto& child = std::get<model::GameEventChildSwitch>(currentChild.value());
        if (propName == "check") {
          std::vector<std::string> parts;
          sdl2w::split(propValue, ":", parts);
          if (parts.size() < 2) {
            throw std::runtime_error("Invalid check property: " + line);
          }
          model::SwitchCase check;
          check.conditionStr = sdl2w::trim(parts[0]);
          check.next = sdl2w::trim(parts[1]);
          child.cases.push_back(check);
        } else if (propName == "default") {
          child.defaultNext = sdl2w::trim(propValue);
        }

        if (child.defaultNext.empty()) {
          throw std::runtime_error("Default next is required for switch: " + line);
        }

        currentChild = child;
      }
      // Handle exec child properties
      else if (std::holds_alternative<model::GameEventChildExec>(currentChild.value())) {
        auto& child = std::get<model::GameEventChildExec>(currentChild.value());

        if (propName == "p") {
          child.paragraphs.push_back(sdl2w::trim(propValue));
        } else if (propName == "e") {
          child.execStr = propValue; // Keep full value including pipes
        } else if (propName == "n") {
          // Next can be "4" or "1:_questStartText"
          // std::vector<std::string> parts;
          // sdl2w::split(propValue, ":", parts);
          // child.next = sdl2w::trim(parts[0]);
          child.next = sdl2w::trim(propValue);
        }
        currentChild = child;
      }
      // Handle bool child properties
      // else if (std::holds_alternative<model::GameEventChildBool>(currentChild.value()))
      // {
      //   auto& child = std::get<model::GameEventChildBool>(currentChild.value());

      //   if (propName == "check") {
      //     child.checkStr = sdl2w::trim(propValue);
      //   } else if (propName == "pass") {
      //     child.pass = sdl2w::trim(propValue);
      //   } else if (propName == "fail") {
      //     child.fail = sdl2w::trim(propValue);
      //   }
      //   currentChild = child;
      // }
      // Handle end child properties
      else if (std::holds_alternative<model::GameEventChildEnd>(currentChild.value())) {
        auto& child = std::get<model::GameEventChildEnd>(currentChild.value());

        if (propName == "n") {
          child.next = sdl2w::trim(propValue);
        }
        currentChild = child;
      }
    }
  }

  // Save last event and child
  if (inEvent) {
    if (currentChild.has_value()) {
      currentEvent.children.push_back(currentChild.value());
    }
    gameEvents[currentEvent.id] = currentEvent;
  }
}

} // namespace db
