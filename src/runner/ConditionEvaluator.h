#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"

#include <optional>

namespace runner {

struct ConditionEvaluatorFuncs {
  const bmin::Map<String, String>& storage;
  DynArray<String> onceKeysToCommit;

  ConditionEvaluatorFuncs(const bmin::Map<String, String>& storage);

  std::optional<int> getNumFromStorageOrArgInt(const String& a);
  std::optional<double> getNumFromStorageOrArgDouble(const String& a);
  bool isNumber(const String& a);

  bool IS(const String& a);
  bool ISNOT(const String& a);
  bool EQ(const String& a, const String& b);
  bool NEQ(const String& a, const String& b);
  bool GT(const String& a, const String& b);
  bool GTE(const String& a, const String& b);
  bool LT(const String& a, const String& b);
  bool LTE(const String& a, const String& b);
  bool ALL(const DynArray<String>& args);
  bool ANY(const DynArray<String>& args);
  bool ONCE(const String& a);

  bool FUNC_HasItem(const String& itemName);
  bool FUNC_QuestStarted(const String& questName);
  bool FUNC_QuestCompleted(const String& questName);
  bool FUNC_QuestStepEq(const String& questName, const String& stepId);
};

class ConditionEvaluator {
public:
  String baseConditionStr;
  ConditionEvaluatorFuncs funcs;

  ConditionEvaluator(const bmin::Map<String, String>& storage,
                     const String& baseConditionStr);
  void assertFuncArgs(const String& funcName, const DynArray<String>& funcArgs,
                      size_t expectedArgs);
  bool evalFunc(const String& funcName, const DynArray<String>& funcArgs);
  bool evalCondition(const String& str);
};

} // namespace runner
