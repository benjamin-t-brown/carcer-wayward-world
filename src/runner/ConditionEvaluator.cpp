#include "ConditionEvaluator.h"
#include "EventRunnerHelpers.h"
#include <algorithm>
#include <stdexcept>

namespace runner {

ConditionEvaluatorFuncs::ConditionEvaluatorFuncs(const bmin::Map<bmin::String, bmin::String>& storage)
    : storage(storage) {}

std::optional<int> ConditionEvaluatorFuncs::getNumFromStorageOrArgInt(const bmin::String& a) {
  if (bmin::isInt(a)) {
    return bmin::parseInt(a);
  }
  auto v = getStorage(storage, a);
  if (v && bmin::isInt(*v)) {
    return bmin::parseInt(*v);
  }
  return std::nullopt;
}

std::optional<double>
ConditionEvaluatorFuncs::getNumFromStorageOrArgDouble(const bmin::String& a) {
  if (bmin::isDouble(a)) {
    return bmin::parseDouble(a);
  }
  auto v = getStorage(storage, a);
  if (v && bmin::isDouble(*v)) {
    return bmin::parseDouble(*v);
  }
  return std::nullopt;
}

bool ConditionEvaluatorFuncs::isNumber(const bmin::String& a) {
  return getNumFromStorageOrArgDouble(a).has_value();
}

bool ConditionEvaluatorFuncs::IS(const bmin::String& a) {
  if (a == "true") {
    return true;
  }
  if (a == "false") {
    return false;
  }
  const bmin::String vStr = getStorage(storage, a).value_or("");
  if (vStr == "0" || vStr == "false" || vStr.empty()) {
    return false;
  }
  return true;
}

bool ConditionEvaluatorFuncs::ISNOT(const bmin::String& a) { return !IS(a); }

bool ConditionEvaluatorFuncs::EQ(const bmin::String& a, const bmin::String& b) {
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

bool ConditionEvaluatorFuncs::NEQ(const bmin::String& a, const bmin::String& b) { return !EQ(a, b); }

bool ConditionEvaluatorFuncs::GT(const bmin::String& a, const bmin::String& b) {
  auto num1 = getNumFromStorageOrArgDouble(a);
  auto num2 = getNumFromStorageOrArgDouble(b);
  if (!num1 || !num2) {
    return false;
  }

  return *num1 > *num2;
}

bool ConditionEvaluatorFuncs::GTE(const bmin::String& a, const bmin::String& b) {
  return EQ(a, b) || GT(a, b);
}

bool ConditionEvaluatorFuncs::LT(const bmin::String& a, const bmin::String& b) {
  auto num1 = getNumFromStorageOrArgDouble(a);
  auto num2 = getNumFromStorageOrArgDouble(b);
  if (!num1 || !num2) {
    return false;
  }
  return *num1 < *num2;
}

bool ConditionEvaluatorFuncs::LTE(const bmin::String& a, const bmin::String& b) {
  return EQ(a, b) || LT(a, b);
}

bool ConditionEvaluatorFuncs::ALL(const bmin::DynArray<bmin::String>& args) {
  for (const auto& arg : args) {
    if (arg == "false") {
      return false;
    }
    if (arg != "true") {
      throw std::runtime_error(("Invalid argument for ALL: " + arg).cStr());
    }
  }
  return true;
}

bool ConditionEvaluatorFuncs::ANY(const bmin::DynArray<bmin::String>& args) {
  for (const auto& arg : args) {
    if (arg == "true") {
      return true;
    }
    if (arg != "false") {
      throw std::runtime_error(("Invalid argument for ANY: " + arg).cStr());
    }
  }
  return false;
}

bool ConditionEvaluatorFuncs::ONCE(const bmin::String& a) {
  const bmin::String onceKey = "once." + a;
  auto v = getStorage(storage, onceKey);
  if (v && *v == "true") {
    return false;
  }
  if (std::find(onceKeysToCommit.begin(), onceKeysToCommit.end(), onceKey) ==
      onceKeysToCommit.end()) {
    onceKeysToCommit.pushBack(onceKey);
  }
  return true;
}

bool ConditionEvaluatorFuncs::FUNC_HasItem(const bmin::String& itemName) {
  auto v = getStorage(storage, "vars.items." + itemName);
  return v && !v->empty() && *v != "0" && *v != "false";
}

bool ConditionEvaluatorFuncs::FUNC_QuestStarted(const bmin::String& questName) {
  auto v = getStorage(storage, "vars.quests." + questName + ".started");
  return v && !v->empty() && *v != "0" && *v != "false";
}

bool ConditionEvaluatorFuncs::FUNC_QuestCompleted(const bmin::String& questName) {
  auto v = getStorage(storage, "vars.quests." + questName + ".completed");
  return v && !v->empty() && *v != "0" && *v != "false";
}

bool ConditionEvaluatorFuncs::FUNC_QuestStepEq(const bmin::String& questName,
                                               const bmin::String& stepId) {
  auto v = getStorage(storage, "vars.quests." + questName + ".step");
  return v && *v == stepId;
}

ConditionEvaluator::ConditionEvaluator(const bmin::Map<bmin::String, bmin::String>& storage,
                                       const bmin::String& baseConditionStr)
    : baseConditionStr(baseConditionStr), funcs(storage) {}

void ConditionEvaluator::assertFuncArgs(const bmin::String& funcName,
                                        const bmin::DynArray<bmin::String>& funcArgs,
                                        size_t expectedArgs) {
  if (funcArgs.size() != expectedArgs) {
    throw std::runtime_error(("Invalid number of arguments for function '" + funcName +
                              "'. Expected " + bmin::toString(expectedArgs) + ", got " +
                              bmin::toString(funcArgs.size()))
                                 .cStr());
  }
}

bool ConditionEvaluator::evalFunc(const bmin::String& funcName,
                                  const bmin::DynArray<bmin::String>& funcArgs) {
  auto simplifyArg = [this](const bmin::String& arg) -> bmin::String {
    if (isFunctionCall(arg)) {
      FunctionCall call = parseFunctionCall(arg);
      const bool result = evalFunc(call.funcName, call.args);
      return result ? "true" : "false";
    }
    return arg;
  };

  if (funcName == "IS") {
    assertFuncArgs(funcName, funcArgs, 1);
    const bmin::String val = simplifyArg(funcArgs[0]);
    return funcs.IS(val);
  } else if (funcName == "ISNOT") {
    assertFuncArgs(funcName, funcArgs, 1);
    const bmin::String val = simplifyArg(funcArgs[0]);
    return funcs.ISNOT(val);
  } else if (funcName == "EQ") {
    assertFuncArgs(funcName, funcArgs, 2);
    const bmin::String a = simplifyArg(funcArgs[0]);
    const bmin::String b = simplifyArg(funcArgs[1]);
    return funcs.EQ(a, b);
  } else if (funcName == "NEQ") {
    assertFuncArgs(funcName, funcArgs, 2);
    const bmin::String a = simplifyArg(funcArgs[0]);
    const bmin::String b = simplifyArg(funcArgs[1]);
    return funcs.NEQ(a, b);
  } else if (funcName == "GT") {
    assertFuncArgs(funcName, funcArgs, 2);
    const bmin::String a = simplifyArg(funcArgs[0]);
    const bmin::String b = simplifyArg(funcArgs[1]);
    return funcs.GT(a, b);
  } else if (funcName == "GTE") {
    assertFuncArgs(funcName, funcArgs, 2);
    const bmin::String a = simplifyArg(funcArgs[0]);
    const bmin::String b = simplifyArg(funcArgs[1]);
    return funcs.GTE(a, b);
  } else if (funcName == "LT") {
    assertFuncArgs(funcName, funcArgs, 2);
    const bmin::String a = simplifyArg(funcArgs[0]);
    const bmin::String b = simplifyArg(funcArgs[1]);
    return funcs.LT(a, b);
  } else if (funcName == "LTE") {
    assertFuncArgs(funcName, funcArgs, 2);
    const bmin::String a = simplifyArg(funcArgs[0]);
    const bmin::String b = simplifyArg(funcArgs[1]);
    return funcs.LTE(a, b);
  } else if (funcName == "ALL") {
    bmin::DynArray<bmin::String> subArgs;
    for (const auto& arg : funcArgs) {
      subArgs.pushBack(simplifyArg(arg));
    }
    return funcs.ALL(subArgs);
  } else if (funcName == "ANY") {
    bmin::DynArray<bmin::String> subArgs;
    for (const auto& arg : funcArgs) {
      subArgs.pushBack(simplifyArg(arg));
    }
    return funcs.ANY(subArgs);
  } else if (funcName == "ONCE") {
    assertFuncArgs(funcName, funcArgs, 1);
    const bmin::String a = simplifyArg(funcArgs[0]);
    return funcs.ONCE(a);
  } else if (funcName == "HAS_ITEM") {
    assertFuncArgs(funcName, funcArgs, 1);
    const bmin::String itemName = simplifyArg(funcArgs[0]);
    return funcs.FUNC_HasItem(itemName);
  } else if (funcName == "QUEST_IS_STARTED") {
    assertFuncArgs(funcName, funcArgs, 1);
    const bmin::String questName = simplifyArg(funcArgs[0]);
    return funcs.FUNC_QuestStarted(questName);
  } else if (funcName == "QUEST_IS_COMPLETE") {
    assertFuncArgs(funcName, funcArgs, 1);
    const bmin::String questName = simplifyArg(funcArgs[0]);
    return funcs.FUNC_QuestCompleted(questName);
  } else if (funcName == "QUEST_STEP_EQ") {
    assertFuncArgs(funcName, funcArgs, 2);
    const bmin::String questName = simplifyArg(funcArgs[0]);
    const bmin::String stepNum = simplifyArg(funcArgs[1]);
    return funcs.FUNC_QuestStepEq(questName, stepNum);
  }
  throw std::runtime_error(("Conditional function '" + funcName + "' not found.").cStr());
}

bool ConditionEvaluator::evalCondition(const bmin::String& str) {
  const bmin::String conditionStr = trim(str);
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
  }
  throw std::runtime_error(("Invalid condition: " + baseConditionStr).cStr());
}

} // namespace runner
