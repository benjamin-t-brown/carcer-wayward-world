#include "SpecialEventRunner.h"
#include "ConditionEvaluator.h"
#include "EventRunnerHelpers.h"
#include "StringEvaluator.h"
#include <algorithm>
#include <functional>

namespace runner {

static String replaceAll(String str, const String& from, const String& to) {
  size_t pos = 0;
  while ((pos = str.find(from, pos)) != String::npos) {
    str.erase(pos, from.size());
    str.insert(pos, to);
    pos += to.size();
  }
  return str;
}

SpecialEventRunner::SpecialEventRunner(
    const bmin::Map<String, String>& initialStorage,
    const model::GameEvent& gameEvent,
    const bmin::Map<String, model::GameEvent>& gameEvents)
    : storage(initialStorage), gameEvent(gameEvent), gameEvents(gameEvents) {
  reset();
}

void SpecialEventRunner::reset() {
  bool hasRoot = false;
  displayText.clear();
  displayTextChoices.clear();
  errors.clear();
  currentNodeId = "";
  for (const auto& child : gameEvent.children) {
    std::visit(
        [&hasRoot](const auto& node) {
          if (node.id == "root") {
            hasRoot = true;
          }
        },
        child);
  }
  if (hasRoot) {
    currentNodeId = "root";
  } else if (!gameEvent.children.empty()) {
    std::visit([this](const auto& node) { currentNodeId = node.id; },
               gameEvent.children[0]);
  } else {
    currentNodeId = "";
  }
}

std::optional<model::GameEventChild> SpecialEventRunner::getCurrentNode() {
  for (const auto& child : gameEvent.children) {
    bool matches = false;
    std::visit(
        [&matches, this](const auto& node) {
          if (node.id == this->currentNodeId) {
            matches = true;
          }
        },
        child);
    if (matches) {
      return child;
    }
  }
  return std::nullopt;
}

String SpecialEventRunner::getNextNodeId() {
  auto currentNode = getCurrentNode();
  if (!currentNode) {
    return "";
  }
  String nextNodeId;
  std::visit(
      [&nextNodeId](const auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, model::GameEventChildExec>) {
          nextNodeId = node.next;
        }
      },
      *currentNode);
  return nextNodeId;
}

DynArray<model::Variable> SpecialEventRunner::getVarsFromNode() {
  DynArray<model::Variable> vars;
  DynArray<String> usedNodes;

  std::function<void(const model::GameEvent&)> innerHelper =
      [&](const model::GameEvent& event) {
        if (std::find(usedNodes.begin(), usedNodes.end(), event.id) != usedNodes.end()) {
          return;
        }
        usedNodes.pushBack(event.id);

        for (const auto& variable : event.vars) {
          if (variable.importFrom.empty()) {
            vars.pushBack(variable);
          }
        }

        for (const auto& variable : event.vars) {
          if (!variable.importFrom.empty()) {
            for (auto it = gameEvents.begin(); it != gameEvents.end(); ++it) {
              if (it->value.id == variable.importFrom) {
                innerHelper(it->value);
                break;
              }
            }
          }
        }
      };

  innerHelper(gameEvent);
  return vars;
}

String SpecialEventRunner::replaceVariables(const String& text) {
  auto vars = getVarsFromNode();
  String result = trim(text);

  for (const auto& variable : vars) {
    const String placeholder = "@" + variable.key;
    const String value = variable.value;
    result = replaceAll(result, placeholder, value);
  }

  return result;
}

bool SpecialEventRunner::evalExecStr(const String& str) {
  const String trimmed = trim(str);
  if (trimmed.empty()) {
    return true;
  }
  if (trimmed == "true") {
    return true;
  }
  if (trimmed == "false") {
    return false;
  }

  StringEvaluator evaluator(storage, trimmed);
  try {
    evaluator.evalStr(trimmed);
    return true;
  } catch (const std::exception& e) {
    errors.pushBack({currentNodeId, e.what()});
    return false;
  }
}

ConditionResult SpecialEventRunner::evalCondition(const String& conditionStr) {
  ConditionEvaluator evaluator(storage, conditionStr);
  try {
    const bool result = evaluator.evalCondition(conditionStr);
    return {result, evaluator.funcs.onceKeysToCommit};
  } catch (const std::exception& e) {
    errors.pushBack({currentNodeId, e.what()});
    return {false, {}};
  }
}

void SpecialEventRunner::commitOnceKeys(const DynArray<String>& onceKeysToCommit) {
  for (const auto& onceKeyToCommit : onceKeysToCommit) {
    setStorage(storage, onceKeyToCommit, "true");
  }
}

String SpecialEventRunner::joinParagraphs(const bmin::DynArray<String>& paragraphs) {
  if (paragraphs.empty()) {
    return "";
  }
  String result;
  for (size_t i = 0; i < paragraphs.size(); ++i) {
    if (i > 0) {
      result += "\n";
    }
    result += paragraphs[i];
  }
  return result;
}

String SpecialEventRunner::resolveChoiceText(const model::Choice& choice,
                                             DynArray<String>& onceKeysToCommit) {
  for (const auto& switchText : choice.switchText) {
    if (!switchText.conditionStr.empty()) {
      ConditionResult obj = evalCondition(replaceVariables(switchText.conditionStr));
      if (obj.result) {
        for (const auto& key : obj.onceKeysToCommit) {
          onceKeysToCommit.pushBack(key);
        }
        return replaceVariables(switchText.text);
      }
    }
  }
  return replaceVariables(choice.text);
}

void SpecialEventRunner::advance(const String& nodeId,
                                 const DynArray<String>& onceKeysToCommit,
                                 const String& execStr) {
  if (!errors.empty()) {
    return;
  }

  commitOnceKeys(onceKeysToCommit);
  if (!execStr.empty()) {
    auto strLines = splitExecStatements(execStr);
    for (const auto& strLine : strLines) {
      evalExecStr(replaceVariables(strLine));
    }
  }

  displayTextChoices.clear();

  currentNodeId = nodeId;
  auto currentNode = getCurrentNode();
  if (nodeId.empty() || !currentNode) {
    return;
  }

  std::visit(
      [this](const auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, model::GameEventChildExec>) {
          auto strLines = splitExecStatements(node.execStr);
          for (const auto& strLine : strLines) {
            evalExecStr(replaceVariables(strLine));
          }
          displayText = replaceVariables(joinParagraphs(node.paragraphs));
          if (displayText.empty() || node.autoAdvance) {
            advance(node.next, {}, "");
          }
        } else if constexpr (std::is_same_v<T, model::GameEventChildChoice>) {
          const String choiceDisplayText = replaceVariables(node.text);
          if (!choiceDisplayText.empty()) {
            displayText = choiceDisplayText;
          }
          for (const auto& choice : node.choices) {
            ConditionResult obj;
            if (!choice.conditionStr.empty()) {
              obj = evalCondition(replaceVariables(choice.conditionStr));
            } else {
              obj = {true, {}};
            }
            if (obj.result) {
              DisplayTextChoice displayChoice;
              displayChoice.prefix = replaceVariables(choice.prefixText);
              DynArray<String> switchOnceKeys;
              displayChoice.text = resolveChoiceText(choice, switchOnceKeys);
              displayChoice.execStr = replaceVariables(choice.evalStr);
              displayChoice.next = choice.next;
              displayChoice.onceKeysToCommit = obj.onceKeysToCommit;
              for (const auto& key : switchOnceKeys) {
                displayChoice.onceKeysToCommit.pushBack(key);
              }
              displayTextChoices.pushBack(displayChoice);
            }
          }
        } else if constexpr (std::is_same_v<T, model::GameEventChildSwitch>) {
          bool found = false;
          for (const auto& c : node.cases) {
            ConditionResult obj = evalCondition(replaceVariables(c.conditionStr));
            if (obj.result) {
              advance(c.next, obj.onceKeysToCommit, "");
              found = true;
              break;
            }
          }
          if (!found) {
            advance(node.defaultNext, {}, "");
          }
        } else if constexpr (std::is_same_v<T, model::GameEventChildEnd>) {
          displayText = "End.";
        }
      },
      *currentNode);
}

String SpecialEventRunner::storageToString() const {
  DynArray<std::pair<String, String>> pairs;
  pairs.reserve(storage.size());
  auto& mutableStorage = const_cast<bmin::Map<String, String>&>(storage);
  for (auto it = mutableStorage.begin(); it != mutableStorage.end(); ++it) {
    pairs.emplaceBack(it->key, it->value);
  }

  std::sort(pairs.begin(), pairs.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

  String result;
  for (const auto& [key, value] : pairs) {
    result += key + "=" + value + "\n";
  }
  return result;
}

SpecialEventRunnerInterface::SpecialEventRunnerInterface(SpecialEventRunner& runner)
    : runner(runner) {}

void SpecialEventRunnerInterface::startEvent() {
  runner.reset();
  runner.advance(runner.currentNodeId);
}

void SpecialEventRunnerInterface::continueEvent() {
  const String nextNodeId = runner.getNextNodeId();
  if (!nextNodeId.empty()) {
    runner.advance(nextNodeId);
  }
}

void SpecialEventRunnerInterface::selectChoice(int choiceIndex) {
  if (choiceIndex < 0 ||
      static_cast<size_t>(choiceIndex) >= runner.displayTextChoices.size()) {
    return;
  }
  const auto& choice = runner.displayTextChoices[choiceIndex];
  runner.advance(choice.next, choice.onceKeysToCommit, choice.execStr);
}

SpecialEventRunnerInterfaceState SpecialEventRunnerInterface::getState() {
  if (runner.displayText.empty() && runner.displayTextChoices.empty()) {
    return SpecialEventRunnerInterfaceState::WAITING_TO_START;
  }

  if (!runner.displayTextChoices.empty()) {
    return SpecialEventRunnerInterfaceState::WAITING_TO_SELECT_CHOICE;
  }

  if (!runner.displayText.empty()) {
    const String nextNodeId = runner.getNextNodeId();
    if (!nextNodeId.empty()) {
      return SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE;
    }
  }

  if (!runner.displayText.empty()) {
    return SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE;
  }

  return SpecialEventRunnerInterfaceState::WAITING_TO_START;
}

String SpecialEventRunnerInterface::stateToString(SpecialEventRunnerInterfaceState state) {
  switch (state) {
  case SpecialEventRunnerInterfaceState::WAITING_TO_START:
    return "WAITING_TO_START";
  case SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE:
    return "WAITING_TO_CONTINUE";
  case SpecialEventRunnerInterfaceState::WAITING_TO_SELECT_CHOICE:
    return "WAITING_TO_SELECT_CHOICE";
  }
  return "UNKNOWN";
}

} // namespace runner
