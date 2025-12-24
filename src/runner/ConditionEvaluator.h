#pragma once

#include "EventRunnerHelpers.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace runner {

struct ConditionEvaluatorFuncs {
  std::unordered_map<std::string, std::string>& storage;
  std::vector<std::string> onceKeysToCommit;

  ConditionEvaluatorFuncs(std::unordered_map<std::string, std::string>& storage);

  bool IS(const std::string& a);
  bool ISNOT(const std::string& a);
  bool EQ(const std::string& a, const std::string& b);
  bool NEQ(const std::string& a, const std::string& b);
  bool GT(const std::string& a, const std::string& b);
  bool GTE(const std::string& a, const std::string& b);
  bool LT(const std::string& a, const std::string& b);
  bool LTE(const std::string& a, const std::string& b);
  bool ALL(const std::vector<std::string>& args);
  bool ANY(const std::vector<std::string>& args);
  bool ONCE(const std::string& a);
};

class ConditionEvaluator {
public:
  std::string baseConditionStr;
  ConditionEvaluatorFuncs funcs;

  ConditionEvaluator(std::unordered_map<std::string, std::string>& storage,
                     const std::string& baseConditionStr);

  bool evalFunc(const std::string& funcName, const std::vector<std::string>& funcArgs);
  bool evalCondition(const std::string& str);
};

} // namespace runner

