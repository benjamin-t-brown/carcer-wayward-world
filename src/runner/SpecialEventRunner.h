#pragma once

#include "../model/SpecialEvents.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

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
  std::unordered_map<std::string, model::GameEvent> gameEvents;
  std::string currentNodeId;

  std::string displayText;
  // std::string nextNodeId;
  std::vector<DisplayTextChoice> displayTextChoices;
  std::vector<ErrorInfo> errors;

  SpecialEventRunner(const std::unordered_map<std::string, std::string>& initialStorage,
                     const model::GameEvent& gameEvent,
                     const std::unordered_map<std::string, model::GameEvent>& gameEvents);

  void reset();
  std::optional<model::GameEventChild> getCurrentNode();
  std::string getNextNodeId();
  std::string replaceVariables(const std::string& text);
  bool evalExecStr(const std::string& str);
  ConditionResult evalCondition(const std::string& conditionStr);
  void commitOnceKeys(const std::vector<std::string>& onceKeysToCommit);
  void advance(const std::string& nodeId,
               const std::vector<std::string>& onceKeysToCommit = {},
               const std::string& execStr = "");
  std::string storageToString() const;

private:
  std::vector<model::Variable> getVarsFromNode();
  std::string joinParagraphs(const std::vector<std::string>& paragraphs);
};

enum class SpecialEventRunnerInterfaceState {
  WAITING_TO_START,
  WAITING_TO_CONTINUE,
  WAITING_TO_SELECT_CHOICE
};

class SpecialEventRunnerInterface {
  SpecialEventRunner& runner;

public:
  SpecialEventRunnerInterface(SpecialEventRunner& runner);
  void startEvent();
  void continueEvent();
  void selectChoice(int choiceIndex);
  SpecialEventRunnerInterfaceState getState();
  static std::string stateToString(SpecialEventRunnerInterfaceState state);
};

} // namespace runner
