#include "SpecialEventRunner.h"
#include "ConditionEvaluator.h"
#include "EventRunnerHelpers.h"
#include "StringEvaluator.h"
#include <algorithm>
#include <functional>
#include <vector>

namespace runner {

// Helper to replace all occurrences of a substring
static std::string
replaceAll(std::string str, const std::string& from, const std::string& to) {
  size_t pos = 0;
  while ((pos = str.find(from, pos)) != std::string::npos) {
    str.replace(pos, from.length(), to);
    pos += to.length();
  }
  return str;
}

// SpecialEventRunner implementation
SpecialEventRunner::SpecialEventRunner(
    const std::unordered_map<std::string, std::string>& initialStorage,
    const model::GameEvent& gameEvent,
    const std::unordered_map<std::string, model::GameEvent>& gameEvents)
    : storage(initialStorage), gameEvent(gameEvent), gameEvents(gameEvents) {
  reset();
}

void SpecialEventRunner::reset() {
  // Find root node or use first node
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

std::string SpecialEventRunner::getNextNodeId() {
  auto currentNode = getCurrentNode();
  if (!currentNode) {
    return "";
  }
  std::string nextNodeId;
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

std::vector<model::Variable> SpecialEventRunner::getVarsFromNode() {
  std::vector<model::Variable> vars;
  std::vector<std::string> usedNodes;

  std::function<void(const model::GameEvent&)> innerHelper =
      [&](const model::GameEvent& event) {
        // Infinite loop protection
        if (std::find(usedNodes.begin(), usedNodes.end(), event.id) != usedNodes.end()) {
          return;
        }
        usedNodes.push_back(event.id);

        // Add variables without importFrom
        for (const auto& variable : event.vars) {
          if (variable.importFrom.empty()) {
            vars.push_back(variable);
          }
        }

        // Add variables from imported events
        for (const auto& variable : event.vars) {
          if (!variable.importFrom.empty()) {
            for (const auto& [id, sourceEvent] : gameEvents) {
              if (sourceEvent.id == variable.importFrom) {
                innerHelper(sourceEvent);
                break;
              }
            }
          }
        }
      };

  innerHelper(gameEvent);
  return vars;
}

std::string SpecialEventRunner::replaceVariables(const std::string& text) {
  auto vars = getVarsFromNode();
  std::string result = trim(text);

  for (const auto& variable : vars) {
    std::string placeholder = "@" + variable.key;
    std::string value = variable.value;
    result = replaceAll(result, placeholder, value);
  }

  return result;
}

bool SpecialEventRunner::evalExecStr(const std::string& str) {
  std::string trimmed = trim(str);
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
    errors.push_back({currentNodeId, e.what()});
    return false;
  }
}

ConditionResult SpecialEventRunner::evalCondition(const std::string& conditionStr) {
  ConditionEvaluator evaluator(storage, conditionStr);
  try {
    bool result = evaluator.evalCondition(conditionStr);
    return {result, evaluator.funcs.onceKeysToCommit};
  } catch (const std::exception& e) {
    errors.push_back({currentNodeId, e.what()});
    return {false, {}};
  }
}

void SpecialEventRunner::commitOnceKeys(
    const std::vector<std::string>& onceKeysToCommit) {
  for (const auto& onceKeyToCommit : onceKeysToCommit) {
    setStorage(storage, onceKeyToCommit, "true");
  }
}

std::string
SpecialEventRunner::joinParagraphs(const std::vector<std::string>& paragraphs) {
  if (paragraphs.empty()) {
    return "";
  }
  std::string result;
  for (size_t i = 0; i < paragraphs.size(); ++i) {
    if (i > 0) {
      result += "\n";
    }
    result += paragraphs[i];
  }
  return result;
}

void SpecialEventRunner::advance(const std::string& nodeId,
                                 const std::vector<std::string>& onceKeysToCommit,
                                 const std::string& execStr) {
  if (!errors.empty()) {
    return;
  }

  commitOnceKeys(onceKeysToCommit);
  if (!execStr.empty()) {
    auto strLines = splitString(execStr, '\n');
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
          auto strLines = splitString(node.execStr, '\n');
          for (const auto& strLine : strLines) {
            evalExecStr(replaceVariables(strLine));
          }
          displayText = replaceVariables(joinParagraphs(node.paragraphs));
          if (displayText.empty() || node.autoAdvance) {
            advance(node.next, {}, "");
          }
        } else if constexpr (std::is_same_v<T, model::GameEventChildChoice>) {
          const std::string choiceDisplayText = replaceVariables(node.text);
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
              displayChoice.prefix = (replaceVariables(choice.prefixText));
              displayChoice.text = (replaceVariables(choice.text));
              displayChoice.execStr = (replaceVariables(choice.evalStr));
              displayChoice.next = choice.next;
              displayChoice.onceKeysToCommit = obj.onceKeysToCommit;
              displayTextChoices.push_back(displayChoice);
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

std::string SpecialEventRunner::storageToString() const {
  // Collect all key-value pairs into a vector
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.reserve(storage.size());
  for (const auto& [key, value] : storage) {
    pairs.emplace_back(key, value);
  }

  // Sort by key alphabetically
  std::sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
    return a.first < b.first;
  });

  // Build result string from sorted pairs
  std::string result;
  for (const auto& [key, value] : pairs) {
    result += key + "=" + value + "\n";
  }
  return result;
}

// SpecialEventRunnerInterface implementation
SpecialEventRunnerInterface::SpecialEventRunnerInterface(SpecialEventRunner& runner)
    : runner(runner) {}

void SpecialEventRunnerInterface::startEvent() {
  runner.reset();
  runner.advance(runner.currentNodeId);
}

void SpecialEventRunnerInterface::continueEvent() {
  std::string nextNodeId = runner.getNextNodeId();
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
  // If we haven't started processing yet (no display text and no choices)
  if (runner.displayText.empty() && runner.displayTextChoices.empty()) {
    return SpecialEventRunnerInterfaceState::WAITING_TO_START;
  }

  // If we have choices available, we're waiting for a choice selection
  if (!runner.displayTextChoices.empty()) {
    return SpecialEventRunnerInterfaceState::WAITING_TO_SELECT_CHOICE;
  }

  // If we have display text and a next node, we're waiting to continue
  if (!runner.displayText.empty()) {
    std::string nextNodeId = runner.getNextNodeId();
    if (!nextNodeId.empty()) {
      return SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE;
    }
  }

  // Default to waiting to continue if we have display text
  if (!runner.displayText.empty()) {
    return SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE;
  }

  // Fallback to waiting to start
  return SpecialEventRunnerInterfaceState::WAITING_TO_START;
}

std::string
SpecialEventRunnerInterface::stateToString(SpecialEventRunnerInterfaceState state) {
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
