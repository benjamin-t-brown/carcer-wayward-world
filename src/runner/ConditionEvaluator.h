#pragma once

#include "EventRunnerHelpers.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace runner {

struct ConditionEvaluatorFuncs {
  const std::unordered_map<std::string, std::string>& storage;
  std::vector<std::string> onceKeysToCommit;

  ConditionEvaluatorFuncs(const std::unordered_map<std::string, std::string>& storage);

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
  bool FUNC(const std::string& funcName, const std::vector<std::string>& funcArgs);

  bool FUNC_HasItem(const std::string& itemName);
  bool FUNC_QuestStarted(const std::string& questName);
  bool FUNC_QuestCompleted(const std::string& questName);
  bool FUNC_QuestStepEq(const std::string& questName, const std::string& stepId);
};

class ConditionEvaluator {
public:
  std::string baseConditionStr;
  ConditionEvaluatorFuncs funcs;

  ConditionEvaluator(const std::unordered_map<std::string, std::string>& storage,
                     const std::string& baseConditionStr);
  void assertFuncArgs(const std::string& funcName,
                      const std::vector<std::string>& funcArgs,
                      size_t expectedArgs);
  bool evalFunc(const std::string& funcName, const std::vector<std::string>& funcArgs);
  bool evalCondition(const std::string& str);
};

} // namespace runner
