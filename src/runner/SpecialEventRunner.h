#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"
#include "model/templates/SpecialEvents.h"

#include <optional>

namespace runner {

struct ConditionResult {
  bool result;
  DynArray<String> onceKeysToCommit;
};

struct DisplayTextChoice {
  String execStr;
  String text;
  String prefix;
  String next;
  DynArray<String> onceKeysToCommit;
};

struct ErrorInfo {
  String nodeId;
  String message;
};

class SpecialEventRunner {
public:
  bmin::Map<String, String> storage;
  model::GameEvent gameEvent;
  bmin::Map<String, model::GameEvent> gameEvents;
  String currentNodeId;

  String displayText;
  DynArray<DisplayTextChoice> displayTextChoices;
  DynArray<ErrorInfo> errors;

  SpecialEventRunner(const bmin::Map<String, String>& initialStorage,
                     const model::GameEvent& gameEvent,
                     const bmin::Map<String, model::GameEvent>& gameEvents);

  void reset();
  std::optional<model::GameEventChild> getCurrentNode();
  String getNextNodeId();
  String replaceVariables(const String& text);
  bool evalExecStr(const String& str);
  ConditionResult evalCondition(const String& conditionStr);
  void commitOnceKeys(const DynArray<String>& onceKeysToCommit);
  void advance(const String& nodeId,
               const DynArray<String>& onceKeysToCommit = {},
               const String& execStr = "");
  String storageToString() const;

private:
  DynArray<model::Variable> getVarsFromNode();
  String joinParagraphs(const bmin::DynArray<String>& paragraphs);
  String resolveChoiceText(const model::Choice& choice,
                           DynArray<String>& onceKeysToCommit);
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
  static String stateToString(SpecialEventRunnerInterfaceState state);
};

} // namespace runner
