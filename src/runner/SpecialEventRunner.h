#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "../model/SpecialEvents.h"

namespace runner {

struct ConditionResult {
  bool result;
  std::vector<std::string> onceKeysToCommit;
};

struct DisplayTextChoice {
  std::string execStr;
  std::string text;
  std::string prefix;
  std::string next;
  std::vector<std::string> onceKeysToCommit;
};

struct ErrorInfo {
  std::string nodeId;
  std::string message;
};

class SpecialEventRunner {
public:
  std::unordered_map<std::string, std::string> storage;
  model::GameEvent gameEvent;
  std::vector<model::GameEvent> gameEvents;
  std::string currentNodeId;

  std::string displayText;
  std::vector<DisplayTextChoice> displayTextChoices;
  std::vector<ErrorInfo> errors;

  SpecialEventRunner(
      const std::unordered_map<std::string, std::string>& initialStorage,
      const model::GameEvent& gameEvent,
      const std::vector<model::GameEvent>& gameEvents);

  std::optional<model::GameEventChild> getCurrentNode();
  std::string replaceVariables(const std::string& text);
  bool evalExecStr(const std::string& str);
  ConditionResult evalCondition(const std::string& conditionStr);
  void commitOnceKeys(const std::vector<std::string>& onceKeysToCommit);
  void advance(const std::string& nextNodeId,
               const std::vector<std::string>& onceKeysToCommit,
               const std::string& execStr);

private:
  std::vector<model::Variable> getVarsFromNode();
  std::string joinParagraphs(const std::vector<std::string>& paragraphs);
};

} // namespace runner

