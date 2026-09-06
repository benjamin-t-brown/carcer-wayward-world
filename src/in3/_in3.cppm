module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>

export module carcer.in3;
export import bmin.containers;
export import carcer.data;
import bmin.string_interop;

export {

// --- from runner/ConditionEvaluator.h ---
namespace in3 {

struct ConditionEvaluatorFuncs {
  const bmin::Map<bmin::String, bmin::String>& storage;
  bmin::DynArray<bmin::String> onceKeysToCommit;

  ConditionEvaluatorFuncs(const bmin::Map<bmin::String, bmin::String>& storage);

  std::optional<int> getNumFromStorageOrArgInt(const bmin::String& a);
  std::optional<double> getNumFromStorageOrArgDouble(const bmin::String& a);
  bool isNumber(const bmin::String& a);

  bool IS(const bmin::String& a);
  bool ISNOT(const bmin::String& a);
  bool EQ(const bmin::String& a, const bmin::String& b);
  bool NEQ(const bmin::String& a, const bmin::String& b);
  bool GT(const bmin::String& a, const bmin::String& b);
  bool GTE(const bmin::String& a, const bmin::String& b);
  bool LT(const bmin::String& a, const bmin::String& b);
  bool LTE(const bmin::String& a, const bmin::String& b);
  bool ALL(const bmin::DynArray<bmin::String>& args);
  bool ANY(const bmin::DynArray<bmin::String>& args);
  bool ONCE(const bmin::String& a);

  bool FUNC_HasItem(const bmin::String& itemName);
  bool FUNC_QuestStarted(const bmin::String& questName);
  bool FUNC_QuestCompleted(const bmin::String& questName);
  bool FUNC_QuestStepEq(const bmin::String& questName, const bmin::String& stepId);
};

class ConditionEvaluator {
public:
  bmin::String baseConditionStr;
  ConditionEvaluatorFuncs funcs;

  ConditionEvaluator(const bmin::Map<bmin::String, bmin::String>& storage,
                     const bmin::String& baseConditionStr);
  void assertFuncArgs(const bmin::String& funcName,
                      const bmin::DynArray<bmin::String>& funcArgs,
                      size_t expectedArgs);
  bool evalFunc(const bmin::String& funcName,
                const bmin::DynArray<bmin::String>& funcArgs);
  bool evalCondition(const bmin::String& str);
};

} // namespace in3

// --- from runner/EventRunnerHelpers.h ---
namespace in3 {

// Helper functions for storage (flat map, no nesting)
void setStorage(bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& key,
                const bmin::String& value);

std::optional<bmin::String> getStorage(const bmin::Map<bmin::String, bmin::String>& storage,
                                       const bmin::String& key);

// Drop per-conversation scratch keys (prefix "tmp."). Keeps once.tmp.* and vars.*.
void clearTmpStorageKeys(bmin::Map<bmin::String, bmin::String>& storage);

// Split exec/eval strings into statements (newlines or semicolons, not inside parens)
bmin::DynArray<bmin::String> splitExecStatements(const bmin::String& str);

// Helper to split string by delimiter
bmin::DynArray<bmin::String> splitString(const bmin::String& str, char delimiter);

// Helper to trim whitespace
bmin::String trim(const bmin::String& str);

// Parse function call: "FUNC_NAME(arg1, arg2, ...)"
struct FunctionCall {
  bmin::String funcName;
  bmin::DynArray<bmin::String> args;
};

FunctionCall parseFunctionCall(const bmin::String& str);

bool isFunctionCall(const bmin::String& str);

} // namespace in3

// --- from runner/SpecialEventRunner.h ---
namespace in3 {

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

} // namespace in3

// --- from runner/StringEvaluator.h ---
namespace in3 {

struct StringEvaluatorFuncs {
  bmin::Map<bmin::String, bmin::String>& storage;

  StringEvaluatorFuncs(bmin::Map<bmin::String, bmin::String>& storage);

  bmin::String GET(const bmin::String& a);
  void SET_BOOL(const bmin::String& a, const bmin::String& b);
  void SET_NUM(const bmin::String& a, const bmin::String& b);
  void MOD_NUM(const bmin::String& a, const bmin::String& b);
  void SET_STR(const bmin::String& a, const bmin::String& b);
  void SETUP_DISPOSITION(const bmin::String& characterName);
  void START_QUEST(const bmin::String& questName);
  void COMPLETE_QUEST_STEP(const bmin::String& questName, const bmin::String& stepId);
  void COMPLETE_QUEST(const bmin::String& questName);
  void SPAWN_CH(const bmin::String& chName);
  void DESPAWN_CH(const bmin::String& chName);
  void CHANGE_TILE_AT(const bmin::String& x, const bmin::String& y, const bmin::String& tileName);
  void TELEPORT_TO(const bmin::String& x, const bmin::String& y, const bmin::String& mapName);
  void ADD_ITEM_AT(const bmin::String& x, const bmin::String& y, const bmin::String& itemName);
  void REMOVE_ITEM_AT(const bmin::String& x, const bmin::String& y, const bmin::String& itemName);
  void ADD_ITEM_TO_PLAYER(const bmin::String& itemName);
  void REMOVE_ITEM_FROM_PLAYER(const bmin::String& itemName);
  void OPEN_SHOP(const bmin::String& shopName);
};

class StringEvaluator {
public:
  bmin::String baseStringStr;
  StringEvaluatorFuncs funcs;
  bmin::String strResult;

  StringEvaluator(bmin::Map<bmin::String, bmin::String>& storage,
                  const bmin::String& baseStringStr);

  void assertFuncArgs(const bmin::String& funcName,
                      const bmin::DynArray<bmin::String>& funcArgs,
                      size_t expectedArgs);
  void evalStr(const bmin::String& str);
};

} // namespace in3

} // export
