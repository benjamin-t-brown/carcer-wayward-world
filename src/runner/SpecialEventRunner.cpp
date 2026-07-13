#include "SpecialEventRunner.h"
#include "ConditionEvaluator.h"
#include "EventRunnerHelpers.h"
#include "StringEvaluator.h"
#include <algorithm>
#include <functional>

namespace runner {

static bmin::String replaceAll(bmin::String str, const bmin::String& from, const bmin::String& to) {
  size_t pos = 0;
  while ((pos = str.find(from, pos)) != bmin::String::npos) {
    str.erase(pos, from.size());
    str.insert(pos, to);
    pos += to.size();
  }
  return str;
}

SpecialEventRunner::SpecialEventRunner(
    const bmin::Map<bmin::String, bmin::String>& initialStorage,
    const model::GameEvent& gameEvent,
    const bmin::Map<bmin::String, model::GameEvent>& gameEvents)
    : storage(initialStorage), gameEvent(gameEvent), gameEvents(gameEvents) {
  reset();
}

void SpecialEventRunner::reset() {
  bool hasRoot = false;
  displayText.clear();
  displayTextChoices.clear();
  autoAdvancedText.clear();
  chosenChoiceKeys.clear();
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

std::optional<model::GameEventChild> SpecialEventRunner::getCurrentNode() const {
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

bool SpecialEventRunner::isAtEndNode() const {
  auto currentNode = getCurrentNode();
  if (!currentNode) {
    return false;
  }
  return std::holds_alternative<model::GameEventChildEnd>(*currentNode);
}

bmin::String SpecialEventRunner::getNextNodeId() {
  auto currentNode = getCurrentNode();
  if (!currentNode) {
    return "";
  }
  bmin::String nextNodeId;
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

bmin::DynArray<model::Variable> SpecialEventRunner::getVarsFromNode() {
  bmin::DynArray<model::Variable> vars;
  bmin::DynArray<bmin::String> usedNodes;

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

bmin::String SpecialEventRunner::replaceVariables(const bmin::String& text) {
  auto vars = getVarsFromNode();
  bmin::String result = trim(text);

  for (const auto& variable : vars) {
    const bmin::String placeholder = "@" + variable.key;
    const bmin::String value = variable.value;
    result = replaceAll(result, placeholder, value);
  }

  return result;
}

bool SpecialEventRunner::evalExecStr(const bmin::String& str) {
  const bmin::String trimmed = trim(str);
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

ConditionResult SpecialEventRunner::evalCondition(const bmin::String& conditionStr) {
  ConditionEvaluator evaluator(storage, conditionStr);
  try {
    const bool result = evaluator.evalCondition(conditionStr);
    return {result, evaluator.funcs.onceKeysToCommit};
  } catch (const std::exception& e) {
    errors.pushBack({currentNodeId, e.what()});
    return {false, {}};
  }
}

void SpecialEventRunner::commitOnceKeys(const bmin::DynArray<bmin::String>& onceKeysToCommit) {
  for (const auto& onceKeyToCommit : onceKeysToCommit) {
    setStorage(storage, onceKeyToCommit, "true");
  }
}

bmin::String SpecialEventRunner::joinParagraphs(const bmin::DynArray<bmin::String>& paragraphs) {
  if (paragraphs.empty()) {
    return "";
  }
  bmin::String result;
  for (size_t i = 0; i < paragraphs.size(); ++i) {
    if (i > 0) {
      result += "\n";
    }
    result += paragraphs[i];
  }
  return result;
}

bmin::String SpecialEventRunner::joinDisplaySegments(const bmin::String& earlier,
                                                     const bmin::String& later) {
  if (earlier.empty()) {
    return later;
  }
  if (later.empty()) {
    return earlier;
  }
  return earlier + "\n\n" + later;
}

bool SpecialEventRunner::wasChoiceChosen(const bmin::String& choiceKey) const {
  if (choiceKey.empty()) {
    return false;
  }
  for (const auto& key : chosenChoiceKeys) {
    if (key == choiceKey) {
      return true;
    }
  }
  return false;
}

void SpecialEventRunner::markChoiceChosen(const bmin::String& choiceKey) {
  if (choiceKey.empty() || wasChoiceChosen(choiceKey)) {
    return;
  }
  chosenChoiceKeys.pushBack(choiceKey);
}

bmin::String SpecialEventRunner::resolveChoiceText(const model::Choice& choice,
                                             bmin::DynArray<bmin::String>& onceKeysToCommit) {
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

void SpecialEventRunner::advance(const bmin::String& nodeId,
                                 const bmin::DynArray<bmin::String>& onceKeysToCommit,
                                 const bmin::String& execStr) {
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
          const bmin::String nodeText = replaceVariables(joinParagraphs(node.paragraphs));
          // MODAL: wait for Continue whenever there is text; ignore autoAdvance.
          // TALK (and others): keep authored autoAdvance behavior, but preserve
          // non-empty EXEC text across the auto-advance chain until the next stop.
          const bool shouldAutoAdvance =
              nodeText.empty() ||
              (gameEvent.eventType != model::GameEventType::MODAL && node.autoAdvance);
          if (shouldAutoAdvance) {
            if (!nodeText.empty()) {
              autoAdvancedText = joinDisplaySegments(autoAdvancedText, nodeText);
            }
            advance(node.next, {}, "");
          } else {
            displayText = joinDisplaySegments(autoAdvancedText, nodeText);
            autoAdvancedText.clear();
            // TALK has no Continue button — surface a choice so the player can advance.
            if (gameEvent.eventType == model::GameEventType::TALK && !node.next.empty()) {
              DisplayTextChoice continueChoice;
              continueChoice.text = "(Continue.)";
              continueChoice.next = node.next;
              continueChoice.choiceKey = currentNodeId + ":continue";
              displayTextChoices.pushBack(continueChoice);
            }
          }
        } else if constexpr (std::is_same_v<T, model::GameEventChildChoice>) {
          const bmin::String choiceDisplayText = replaceVariables(node.text);
          // Flush auto-advanced EXEC text into this stop. If both are empty, keep
          // whatever displayText was already showing (e.g. MODAL continue into an
          // empty-text CHOICE menu).
          if (!autoAdvancedText.empty() || !choiceDisplayText.empty()) {
            displayText = joinDisplaySegments(autoAdvancedText, choiceDisplayText);
          }
          autoAdvancedText.clear();
          for (int authoredIndex = 0; authoredIndex < static_cast<int>(node.choices.size());
               authoredIndex++) {
            const auto& choice = node.choices[authoredIndex];
            ConditionResult obj;
            if (!choice.conditionStr.empty()) {
              obj = evalCondition(replaceVariables(choice.conditionStr));
            } else {
              obj = {true, {}};
            }
            if (obj.result) {
              DisplayTextChoice displayChoice;
              displayChoice.prefix = replaceVariables(choice.prefixText);
              bmin::DynArray<bmin::String> switchOnceKeys;
              displayChoice.text = resolveChoiceText(choice, switchOnceKeys);
              displayChoice.execStr = replaceVariables(choice.evalStr);
              displayChoice.next = choice.next;
              displayChoice.onceKeysToCommit = obj.onceKeysToCommit;
              for (const auto& key : switchOnceKeys) {
                displayChoice.onceKeysToCommit.pushBack(key);
              }
              displayChoice.choiceKey =
                  currentNodeId + ":" + bmin::toString(authoredIndex);
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
          // TALK closes immediately on END — no "End." interstitial.
          if (gameEvent.eventType == model::GameEventType::TALK) {
            displayText.clear();
            displayTextChoices.clear();
            autoAdvancedText.clear();
          } else {
            displayText = joinDisplaySegments(autoAdvancedText, "End.");
            autoAdvancedText.clear();
          }
        }
      },
      *currentNode);
}

bmin::String SpecialEventRunner::storageToString() const {
  bmin::DynArray<std::pair<bmin::String, bmin::String>> pairs;
  pairs.reserve(storage.size());
  auto& mutableStorage = const_cast<bmin::Map<bmin::String, bmin::String>&>(storage);
  for (auto it = mutableStorage.begin(); it != mutableStorage.end(); ++it) {
    pairs.emplaceBack(it->key, it->value);
  }

  std::sort(pairs.begin(), pairs.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

  bmin::String result;
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
  const bmin::String nextNodeId = runner.getNextNodeId();
  if (!nextNodeId.empty()) {
    runner.advance(nextNodeId);
  }
}

void SpecialEventRunnerInterface::selectChoice(int choiceIndex) {
  if (choiceIndex < 0 ||
      static_cast<size_t>(choiceIndex) >= runner.displayTextChoices.size()) {
    return;
  }
  const auto choice = runner.displayTextChoices[choiceIndex];
  runner.markChoiceChosen(choice.choiceKey);
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
    const bmin::String nextNodeId = runner.getNextNodeId();
    if (!nextNodeId.empty()) {
      return SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE;
    }
  }

  if (!runner.displayText.empty()) {
    return SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE;
  }

  return SpecialEventRunnerInterfaceState::WAITING_TO_START;
}

bmin::String SpecialEventRunnerInterface::stateToString(SpecialEventRunnerInterfaceState state) {
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
