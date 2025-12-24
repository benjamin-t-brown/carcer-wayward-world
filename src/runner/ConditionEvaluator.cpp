#include "ConditionEvaluator.h"
#include <algorithm>
#include <stdexcept>

namespace runner {

ConditionEvaluatorFuncs::ConditionEvaluatorFuncs(
    std::unordered_map<std::string, std::string>& storage)
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
    std::unordered_map<std::string, std::string>& storage,
    const std::string& baseConditionStr)
    : baseConditionStr(baseConditionStr), funcs(storage) {}

bool ConditionEvaluator::evalFunc(const std::string& funcName,
                                   const std::vector<std::string>& funcArgs) {
  if (funcName == "IS") {
    if (funcArgs.size() != 1) {
      throw std::runtime_error("IS requires 1 argument");
    }
    return funcs.IS(funcArgs[0]);
  } else if (funcName == "ISNOT") {
    if (funcArgs.size() != 1) {
      throw std::runtime_error("ISNOT requires 1 argument");
    }
    return funcs.ISNOT(funcArgs[0]);
  } else if (funcName == "EQ") {
    if (funcArgs.size() != 2) {
      throw std::runtime_error("EQ requires 2 arguments");
    }
    return funcs.EQ(funcArgs[0], funcArgs[1]);
  } else if (funcName == "NEQ") {
    if (funcArgs.size() != 2) {
      throw std::runtime_error("NEQ requires 2 arguments");
    }
    return funcs.NEQ(funcArgs[0], funcArgs[1]);
  } else if (funcName == "GT") {
    if (funcArgs.size() != 2) {
      throw std::runtime_error("GT requires 2 arguments");
    }
    return funcs.GT(funcArgs[0], funcArgs[1]);
  } else if (funcName == "GTE") {
    if (funcArgs.size() != 2) {
      throw std::runtime_error("GTE requires 2 arguments");
    }
    return funcs.GTE(funcArgs[0], funcArgs[1]);
  } else if (funcName == "LT") {
    if (funcArgs.size() != 2) {
      throw std::runtime_error("LT requires 2 arguments");
    }
    return funcs.LT(funcArgs[0], funcArgs[1]);
  } else if (funcName == "LTE") {
    if (funcArgs.size() != 2) {
      throw std::runtime_error("LTE requires 2 arguments");
    }
    return funcs.LTE(funcArgs[0], funcArgs[1]);
  } else if (funcName == "ALL") {
    return funcs.ALL(funcArgs);
  } else if (funcName == "ANY") {
    return funcs.ANY(funcArgs);
  } else if (funcName == "ONCE") {
    if (funcArgs.size() != 1) {
      throw std::runtime_error("ONCE requires 1 argument");
    }
    return funcs.ONCE(funcArgs[0]);
  } else {
    throw std::runtime_error("Conditional function '" + funcName + "' not found.");
  }
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

