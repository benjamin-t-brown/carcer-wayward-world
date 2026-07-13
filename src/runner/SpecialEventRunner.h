#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/SpecialEvents.h"

#include <optional>

namespace runner {

struct ConditionResult {
  bool result;
  bmin::DynArray<bmin::String> onceKeysToCommit;
};

struct DisplayTextChoice {
  bmin::String execStr;
  bmin::String text;
  bmin::String prefix;
  bmin::String next;
  bmin::DynArray<bmin::String> onceKeysToCommit;
  // Stable id for this conversation: "<nodeId>:<authoredChoiceIndex>" (or ":continue").
  bmin::String choiceKey;
};

struct ErrorInfo {
  bmin::String nodeId;
  bmin::String message;
};

class SpecialEventRunner {
public:
  bmin::Map<bmin::String, bmin::String> storage;
  model::GameEvent gameEvent;
  bmin::Map<bmin::String, model::GameEvent> gameEvents;
  bmin::String currentNodeId;

  bmin::String displayText;
  bmin::DynArray<DisplayTextChoice> displayTextChoices;
  bmin::DynArray<ErrorInfo> errors;
  // Choice keys selected earlier in this conversation (for dimming repeats).
  bmin::DynArray<bmin::String> chosenChoiceKeys;

  SpecialEventRunner(const bmin::Map<bmin::String, bmin::String>& initialStorage,
                     const model::GameEvent& gameEvent,
                     const bmin::Map<bmin::String, model::GameEvent>& gameEvents);

  void reset();
  std::optional<model::GameEventChild> getCurrentNode() const;
  bmin::String getNextNodeId();
  bmin::String replaceVariables(const bmin::String& text);
  bool evalExecStr(const bmin::String& str);
  ConditionResult evalCondition(const bmin::String& conditionStr);
  void commitOnceKeys(const bmin::DynArray<bmin::String>& onceKeysToCommit);
  void advance(const bmin::String& nodeId,
               const bmin::DynArray<bmin::String>& onceKeysToCommit = {},
               const bmin::String& execStr = "");
  bmin::String storageToString() const;
  bool wasChoiceChosen(const bmin::String& choiceKey) const;
  void markChoiceChosen(const bmin::String& choiceKey);
  bool isAtEndNode() const;

private:
  // EXEC text queued while auto-advancing; flushed into displayText at the next stop.
  bmin::String autoAdvancedText;

  bmin::DynArray<model::Variable> getVarsFromNode();
  bmin::String joinParagraphs(const bmin::DynArray<bmin::String>& paragraphs);
  bmin::String resolveChoiceText(const model::Choice& choice,
                                 bmin::DynArray<bmin::String>& onceKeysToCommit);
  static bmin::String joinDisplaySegments(const bmin::String& earlier,
                                          const bmin::String& later);
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
  static bmin::String stateToString(SpecialEventRunnerInterfaceState state);
};

} // namespace runner
