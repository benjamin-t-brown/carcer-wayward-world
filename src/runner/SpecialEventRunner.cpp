#include "SpecialEventRunner.h"
#include "ConditionEvaluator.h"
#include "StringEvaluator.h"
#include "EventRunnerHelpers.h"
#include <algorithm>
#include <functional>

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
    const std::vector<model::GameEvent>& gameEvents)
    : storage(initialStorage), gameEvent(gameEvent), gameEvents(gameEvents) {
  // Find root node or use first node
  bool hasRoot = false;
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
            for (const auto& sourceEvent : gameEvents) {
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

std::string SpecialEventRunner::replaceVariables(const std::string& text,
                                                 bool highlight) {
  auto vars = getVarsFromNode();
  std::string result = trim(text);

  for (const auto& variable : vars) {
    std::string placeholder = "@" + variable.key;
    std::string value = variable.value;
    result = replaceAll(result, placeholder, value);
    if (highlight) {
      // In C++ we don't add HTML, just use the value
      // (user said to remove HTML parts)
    }
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

void SpecialEventRunner::advance(const std::string& nextNodeId,
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

  displayText = "";
  displayTextChoices.clear();

  currentNodeId = nextNodeId;
  auto currentNode = getCurrentNode();
  if (nextNodeId.empty() || !currentNode) {
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
          displayText = replaceVariables(joinParagraphs(node.paragraphs), true);
          if (displayText.empty() || node.autoAdvance) {
            advance(node.next, {}, "");
          }
        } else if constexpr (std::is_same_v<T, model::GameEventChildChoice>) {
          displayText = replaceVariables(node.text, true);
          for (const auto& choice : node.choices) {
            ConditionResult obj;
            if (!choice.conditionStr.empty()) {
              obj = evalCondition(replaceVariables(choice.conditionStr));
            } else {
              obj = {true, {}};
            }
            if (obj.result) {
              DisplayTextChoice displayChoice;
              displayChoice.prefix = (replaceVariables(choice.prefixText, false));
              displayChoice.text = (replaceVariables(choice.text, true));
              displayChoice.execStr = (replaceVariables(choice.evalStr, false));
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
        }
        // GameEventChildEnd doesn't need special handling
      },
      *currentNode);
}

} // namespace runner
