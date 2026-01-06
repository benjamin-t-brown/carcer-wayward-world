#include "ConditionEvaluator.h"
#include <algorithm>
#include <stdexcept>

namespace runner {

ConditionEvaluatorFuncs::ConditionEvaluatorFuncs(
    const std::unordered_map<std::string, std::string>& storage)
    : storage(storage) {}

bool ConditionEvaluatorFuncs::IS(const std::string& a) {
  auto v = getStorage(storage, a);
  if (v && *v == "false") {
    return false;
  }
  return v.has_value() && !v->empty() && *v != "0";
}

bool ConditionEvaluatorFuncs::ISNOT(const std::string& a) {
  auto v = getStorage(storage, a);
  return !v.has_value() || v->empty() || *v == "0" || *v == "false";
}

bool ConditionEvaluatorFuncs::EQ(const std::string& a, const std::string& b) {
  auto v1 = getStorage(storage, a);
  auto v2 = getStorage(storage, b);
  if (!v1 && !v2) {
    return true;
  }
  if (!v1 || !v2) {
    return false;
  }
  return *v1 == *v2;
}

bool ConditionEvaluatorFuncs::NEQ(const std::string& a, const std::string& b) {
  return !EQ(a, b);
}

bool ConditionEvaluatorFuncs::GT(const std::string& a, const std::string& b) {
  auto v1 = getStorage(storage, a);
  if (!v1) {
    return false;
  }
  try {
    double num1 = std::stod(*v1);
    double num2 = std::stod(b);
    return num1 > num2;
  } catch (...) {
    return false;
  }
}

bool ConditionEvaluatorFuncs::GTE(const std::string& a, const std::string& b) {
  auto v1 = getStorage(storage, a);
  if (!v1) {
    return false;
  }
  try {
    double num1 = std::stod(*v1);
    double num2 = std::stod(b);
    return num1 >= num2;
  } catch (...) {
    return false;
  }
}

bool ConditionEvaluatorFuncs::LT(const std::string& a, const std::string& b) {
  auto v1 = getStorage(storage, a);
  if (!v1) {
    return false;
  }
  try {
    double num1 = std::stod(*v1);
    double num2 = std::stod(b);
    return num1 < num2;
  } catch (...) {
    return false;
  }
}

bool ConditionEvaluatorFuncs::LTE(const std::string& a, const std::string& b) {
  auto v1 = getStorage(storage, a);
  if (!v1) {
    return false;
  }
  try {
    double num1 = std::stod(*v1);
    double num2 = std::stod(b);
    return num1 <= num2;
  } catch (...) {
    return false;
  }
}

bool ConditionEvaluatorFuncs::ALL(const std::vector<std::string>& args) {
  for (const auto& arg : args) {
    auto v = getStorage(storage, arg);
    if (!v || v->empty() || *v == "0" || *v == "false") {
      return false;
    }
  }
  return true;
}

bool ConditionEvaluatorFuncs::ANY(const std::vector<std::string>& args) {
  for (const auto& arg : args) {
    auto v = getStorage(storage, arg);
    if (v && !v->empty() && *v != "0" && *v != "false") {
      return true;
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
  if (funcName == "IS") {
    assertFuncArgs(funcName, funcArgs, 1);
    return funcs.IS(funcArgs[0]);
  } else if (funcName == "ISNOT") {
    assertFuncArgs(funcName, funcArgs, 1);
    return funcs.ISNOT(funcArgs[0]);
  } else if (funcName == "EQ") {
    assertFuncArgs(funcName, funcArgs, 2);
    return funcs.EQ(funcArgs[0], funcArgs[1]);
  } else if (funcName == "NEQ") {
    assertFuncArgs(funcName, funcArgs, 2);
    return funcs.NEQ(funcArgs[0], funcArgs[1]);
  } else if (funcName == "GT") {
    assertFuncArgs(funcName, funcArgs, 2);
    return funcs.GT(funcArgs[0], funcArgs[1]);
  } else if (funcName == "GTE") {
    assertFuncArgs(funcName, funcArgs, 2);
    return funcs.GTE(funcArgs[0], funcArgs[1]);
  } else if (funcName == "LT") {
    assertFuncArgs(funcName, funcArgs, 2);
    return funcs.LT(funcArgs[0], funcArgs[1]);
  } else if (funcName == "LTE") {
    assertFuncArgs(funcName, funcArgs, 2);
    return funcs.LTE(funcArgs[0], funcArgs[1]);
  } else if (funcName == "ALL") {
    return funcs.ALL(funcArgs);
  } else if (funcName == "ANY") {
    return funcs.ANY(funcArgs);
  } else if (funcName == "ONCE") {
    assertFuncArgs(funcName, funcArgs, 1);
    return funcs.ONCE(funcArgs[0]);
  } else if (funcName == "FUNC") {
    assertFuncArgs(funcName, funcArgs, 1);
    const std::string& subFuncName = funcArgs[0];
    if (subFuncName == "HasItem") {
      assertFuncArgs(funcName + "." + subFuncName, funcArgs, 2);
      return funcs.FUNC_HasItem(funcArgs[1]);
    }
    if (subFuncName == "QuestStarted") {
      assertFuncArgs(funcName + "." + subFuncName, funcArgs, 2);
      return funcs.FUNC_QuestStarted(funcArgs[1]);
    }
    if (subFuncName == "QuestCompleted") {
      assertFuncArgs(funcName + "." + subFuncName, funcArgs, 2);
      return funcs.FUNC_QuestCompleted(funcArgs[1]);
    }
    if (subFuncName == "QuestStepEq") {
      assertFuncArgs(funcName + "." + subFuncName, funcArgs, 3);
      return funcs.FUNC_QuestStepEq(funcArgs[1], funcArgs[2]);
    } else {
      throw std::runtime_error("Conditional FUNC sub function '" + subFuncName +
                               "' not found.");
    }
  }
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
