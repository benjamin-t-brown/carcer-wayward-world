#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "lib/bmin/Map.h"

#include <optional>

namespace runner {

struct ConditionEvaluatorFuncs {
  const bmin::Map<bmin::String, bmin::String>& storage;
  bmin::DynArray<bmin::String> onceKeysToCommit;

  ConditionEvaluatorFuncs(const bmin::Map<bmin::String, bmin::String>& storage);

  std::optional<int> getNumFromStorageOrArgInt(const bmin::String& a);
  std::optional<double> getNumFromStorageOrArgDouble(const bmin::String& a);
  bool isNumber(const bmin::String& a);

  bool IS(const bmin::String& a);
  bool ISNOT(const bmin::String& a);
  bool EQ(const bmin::String& a, const bmin::String& b);
  bool NEQ(const bmin::String& a, const bmin::String& b);
  bool GT(const bmin::String& a, const bmin::String& b);
  bool GTE(const bmin::String& a, const bmin::String& b);
  bool LT(const bmin::String& a, const bmin::String& b);
  bool LTE(const bmin::String& a, const bmin::String& b);
  bool ALL(const bmin::DynArray<bmin::String>& args);
  bool ANY(const bmin::DynArray<bmin::String>& args);
  bool ONCE(const bmin::String& a);

  bool FUNC_HasItem(const bmin::String& itemName);
  bool FUNC_QuestStarted(const bmin::String& questName);
  bool FUNC_QuestCompleted(const bmin::String& questName);
  bool FUNC_QuestStepEq(const bmin::String& questName, const bmin::String& stepId);
};

class ConditionEvaluator {
public:
  bmin::String baseConditionStr;
  ConditionEvaluatorFuncs funcs;

  ConditionEvaluator(const bmin::Map<bmin::String, bmin::String>& storage,
                     const bmin::String& baseConditionStr);
  void assertFuncArgs(const bmin::String& funcName,
                      const bmin::DynArray<bmin::String>& funcArgs,
                      size_t expectedArgs);
  bool evalFunc(const bmin::String& funcName,
                const bmin::DynArray<bmin::String>& funcArgs);
  bool evalCondition(const bmin::String& str);
};

} // namespace runner
