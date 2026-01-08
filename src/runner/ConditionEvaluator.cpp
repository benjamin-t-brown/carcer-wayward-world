#include "ConditionEvaluator.h"
#include <algorithm>
#include <stdexcept>

namespace runner {

ConditionEvaluatorFuncs::ConditionEvaluatorFuncs(
    const std::unordered_map<std::string, std::string>& storage)
    : storage(storage) {}

std::optional<int>
ConditionEvaluatorFuncs::getNumFromStorageOrArgInt(const std::string& a) {
  // Try parsing 'a' directly as int
  try {
    size_t idx;
    int num = std::stoi(a, &idx);
    if (idx == a.size()) {
      return num;
    }
  } catch (...) {
    // Not a number, fall through to storage lookup
  }
  auto v = getStorage(storage, a);
  if (v) {
    try {
      return std::stoi(*v);
    } catch (...) {
      // storage doesn't exist, fall back to 0
    }
  }
  return std::nullopt;
}

std::optional<double>
ConditionEvaluatorFuncs::getNumFromStorageOrArgDouble(const std::string& a) {
  // Try parsing 'a' directly as double
  try {
    size_t idx;
    double num = std::stod(a, &idx);
    if (idx == a.size()) {
      // Successfully parsed whole string as double
      return num;
    }
  } catch (...) {
    // Not a number, fall through to storage lookup
  }
  auto v = getStorage(storage, a);
  if (v) {
    try {
      return std::stod(*v);
    } catch (...) {
      // storage doesn't exist, fall back to 0
    }
  }
  return std::nullopt;
}

bool ConditionEvaluatorFuncs::isNumber(const std::string& a) {
  return getNumFromStorageOrArgDouble(a).has_value();
}

bool ConditionEvaluatorFuncs::IS(const std::string& a) {
  if (a == "true") {
    return true;
  }
  if (a == "false") {
    return false;
  }
  // auto v = getStorage(storage, a);
  std::string vStr = getStorage(storage, a).value_or("");
  if (vStr == "0" || vStr == "false" || vStr.empty()) {
    return false;
  }
  return true;
}

bool ConditionEvaluatorFuncs::ISNOT(const std::string& a) { return !IS(a); }

bool ConditionEvaluatorFuncs::EQ(const std::string& a, const std::string& b) {
  auto aStorage = getStorage(storage, a);
  auto bStorage = getStorage(storage, b);

  if (aStorage && bStorage) {
    return *aStorage == *bStorage;
  } else if (aStorage && !bStorage) {
    if (isNumber(b)) {
      return *aStorage == b;
    }
    return false;
  } else if (!aStorage && bStorage) {
    if (isNumber(a)) {
      return a == *bStorage;
    }
    return false;
  }
  if (isNumber(a) && isNumber(b)) {
    return a == b;
  }
  return false;
}

bool ConditionEvaluatorFuncs::NEQ(const std::string& a, const std::string& b) {
  return !EQ(a, b);
}

bool ConditionEvaluatorFuncs::GT(const std::string& a, const std::string& b) {
  auto num1 = getNumFromStorageOrArgDouble(a);
  auto num2 = getNumFromStorageOrArgDouble(b);
  if (!num1 || !num2) {
    // reject any case that is undefined
    return false;
  }

  return *num1 > *num2;
}

bool ConditionEvaluatorFuncs::GTE(const std::string& a, const std::string& b) {
  return EQ(a, b) || GT(a, b);
}

bool ConditionEvaluatorFuncs::LT(const std::string& a, const std::string& b) {
  auto num1 = getNumFromStorageOrArgDouble(a);
  auto num2 = getNumFromStorageOrArgDouble(b);
  if (!num1 || !num2) {
    // reject any case that is undefined
    return false;
  }
  return *num1 < *num2;
}

bool ConditionEvaluatorFuncs::LTE(const std::string& a, const std::string& b) {
  return EQ(a, b) || LT(a, b);
}

bool ConditionEvaluatorFuncs::ALL(const std::vector<std::string>& args) {
  for (const auto& arg : args) {
    if (arg == "false") {
      return false;
    }
    if (arg != "true") {
      throw std::runtime_error("Invalid argument for ALL: " + arg);
    }
  }
  return true;
}

bool ConditionEvaluatorFuncs::ANY(const std::vector<std::string>& args) {
  for (const auto& arg : args) {
    if (arg == "true") {
      return true;
    }
    if (arg != "false") {
      throw std::runtime_error("Invalid argument for ANY: " + arg);
    }
  }
  return false;
}

bool ConditionEvaluatorFuncs::ONCE(const std::string& a) {
  std::string onceKey = "once." + a;
  auto v = getStorage(storage, onceKey);
  if (v && *v == "true") {
    return false;
  }
  if (std::find(onceKeysToCommit.begin(), onceKeysToCommit.end(), onceKey) ==
      onceKeysToCommit.end()) {
    onceKeysToCommit.push_back(onceKey);
  }
  return true;
}

bool ConditionEvaluatorFuncs::FUNC_HasItem(const std::string& itemName) {
  auto v = getStorage(storage, "vars.items." + itemName);
  return v && !v->empty() && *v != "0" && *v != "false";
}

bool ConditionEvaluatorFuncs::FUNC_QuestStarted(const std::string& questName) {
  auto v = getStorage(storage, "vars.quests." + questName + ".started");
  return v && !v->empty() && *v != "0" && *v != "false";
}

bool ConditionEvaluatorFuncs::FUNC_QuestCompleted(const std::string& questName) {
  auto v = getStorage(storage, "vars.quests." + questName + ".completed");
  return v && !v->empty() && *v != "0" && *v != "false";
}

bool ConditionEvaluatorFuncs::FUNC_QuestStepEq(const std::string& questName,
                                               const std::string& stepId) {
  auto v = getStorage(storage, "vars.quests." + questName + ".step");
  return v && *v == stepId;
}

ConditionEvaluator::ConditionEvaluator(
    const std::unordered_map<std::string, std::string>& storage,
    const std::string& baseConditionStr)
    : baseConditionStr(baseConditionStr), funcs(storage) {}

void ConditionEvaluator::assertFuncArgs(const std::string& funcName,
                                        const std::vector<std::string>& funcArgs,
                                        size_t expectedArgs) {
  if (funcArgs.size() != expectedArgs) {
    throw std::runtime_error("Invalid number of arguments for function '" + funcName +
                             "'. Expected " + std::to_string(expectedArgs) + ", got " +
                             std::to_string(funcArgs.size()));
  }
}

bool ConditionEvaluator::evalFunc(const std::string& funcName,
                                  const std::vector<std::string>& funcArgs) {
  // Helper lambda for recursively evaluating function arguments
  auto simplifyArg = [this](const std::string& arg) -> std::string {
    if (isFunctionCall(arg)) {
      FunctionCall call = parseFunctionCall(arg);
      bool result = evalFunc(call.funcName, call.args);
      return result ? "true" : "false";
    }
    return arg;
  };

  if (funcName == "IS") {
    assertFuncArgs(funcName, funcArgs, 1);
    std::string val = simplifyArg(funcArgs[0]);
    return funcs.IS(val);
  } else if (funcName == "ISNOT") {
    assertFuncArgs(funcName, funcArgs, 1);
    std::string val = simplifyArg(funcArgs[0]);
    return funcs.ISNOT(val);
  } else if (funcName == "EQ") {
    assertFuncArgs(funcName, funcArgs, 2);
    std::string a = simplifyArg(funcArgs[0]);
    std::string b = simplifyArg(funcArgs[1]);
    return funcs.EQ(a, b);
  } else if (funcName == "NEQ") {
    assertFuncArgs(funcName, funcArgs, 2);
    std::string a = simplifyArg(funcArgs[0]);
    std::string b = simplifyArg(funcArgs[1]);
    return funcs.NEQ(a, b);
  } else if (funcName == "GT") {
    assertFuncArgs(funcName, funcArgs, 2);
    std::string a = simplifyArg(funcArgs[0]);
    std::string b = simplifyArg(funcArgs[1]);
    return funcs.GT(a, b);
  } else if (funcName == "GTE") {
    assertFuncArgs(funcName, funcArgs, 2);
    std::string a = simplifyArg(funcArgs[0]);
    std::string b = simplifyArg(funcArgs[1]);
    return funcs.GTE(a, b);
  } else if (funcName == "LT") {
    assertFuncArgs(funcName, funcArgs, 2);
    std::string a = simplifyArg(funcArgs[0]);
    std::string b = simplifyArg(funcArgs[1]);
    return funcs.LT(a, b);
  } else if (funcName == "LTE") {
    assertFuncArgs(funcName, funcArgs, 2);
    std::string a = simplifyArg(funcArgs[0]);
    std::string b = simplifyArg(funcArgs[1]);
    return funcs.LTE(a, b);
  } else if (funcName == "ALL") {
    std::vector<std::string> subArgs;
    for (const auto& arg : funcArgs) {
      subArgs.push_back(simplifyArg(arg));
    }
    return funcs.ALL(subArgs);
  } else if (funcName == "ANY") {
    std::vector<std::string> subArgs;
    for (const auto& arg : funcArgs) {
      subArgs.push_back(simplifyArg(arg));
    }
    return funcs.ANY(subArgs);
  } else if (funcName == "ONCE") {
    assertFuncArgs(funcName, funcArgs, 1);
    std::string a = simplifyArg(funcArgs[0]);
    return funcs.ONCE(a);
  } else if (funcName == "FUNC") {
    assertFuncArgs(funcName, funcArgs, 1);
    std::string subFuncName = simplifyArg(funcArgs[0]);
    if (subFuncName == "HasItem") {
      assertFuncArgs(funcName + "." + subFuncName, funcArgs, 2);
      std::string itemName = simplifyArg(funcArgs[1]);
      return funcs.FUNC_HasItem(itemName);
    }
    if (subFuncName == "QuestStarted") {
      assertFuncArgs(funcName + "." + subFuncName, funcArgs, 2);
      std::string questName = simplifyArg(funcArgs[1]);
      return funcs.FUNC_QuestStarted(questName);
    }
    if (subFuncName == "QuestCompleted") {
      assertFuncArgs(funcName + "." + subFuncName, funcArgs, 2);
      std::string questName = simplifyArg(funcArgs[1]);
      return funcs.FUNC_QuestCompleted(questName);
    }
    if (subFuncName == "QuestStepEq") {
      assertFuncArgs(funcName + "." + subFuncName, funcArgs, 3);
      std::string questName = simplifyArg(funcArgs[1]);
      std::string stepId = simplifyArg(funcArgs[2]);
      return funcs.FUNC_QuestStepEq(questName, stepId);
    } else {
      throw std::runtime_error("Conditional FUNC sub function '" + subFuncName +
                               "' not found.");
    }
  }
  throw std::runtime_error("Conditional function '" + funcName + "' not found.");
  return false;
}

bool ConditionEvaluator::evalCondition(const std::string& str) {
  std::string conditionStr = trim(str);
  if (conditionStr.empty()) {
    return true;
  }
  if (conditionStr == "true") {
    return true;
  }
  if (conditionStr == "false") {
    return false;
  }
  if (isFunctionCall(conditionStr)) {
    FunctionCall call = parseFunctionCall(conditionStr);
    return evalFunc(call.funcName, call.args);
  } else {
    throw std::runtime_error("Invalid condition: " + baseConditionStr);
  }
}

} // namespace runner
