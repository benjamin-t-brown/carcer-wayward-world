#include "StringEvaluator.h"
#include "EventRunnerHelpers.h"
#include <cmath>
#include <stdexcept>

namespace runner {

// Helper function to format number as string: integer if no decimal, otherwise keep decimals
static String formatNumber(double n) {
  double intPart;
  if (std::modf(n, &intPart) == 0.0) {
    return bmin::toString(static_cast<int>(n));
  }

  String result = bmin::toString(n);
  while (!result.empty() && result[result.size() - 1] == '0') {
    result.erase(result.size() - 1, 1);
  }
  if (!result.empty() && result[result.size() - 1] == '.') {
    result.erase(result.size() - 1, 1);
  }
  return result;
}

StringEvaluatorFuncs::StringEvaluatorFuncs(bmin::Map<String, String>& storage)
    : storage(storage) {}

String StringEvaluatorFuncs::GET(const String& a) {
  auto v = getStorage(storage, a);
  return v.value_or("");
}

void StringEvaluatorFuncs::SET_BOOL(const String& a, const String& b) {
  bool v = true;
  if (b == "true") {
    v = true;
  } else if (b == "false") {
    v = false;
  } else {
    v = true;
  }
  setStorage(storage, a, v ? "true" : "false");
}

void StringEvaluatorFuncs::SET_NUM(const String& a, const String& b) {
  if (!bmin::isDouble(b)) {
    throw std::runtime_error(("Invalid number value: " + b).cStr());
  }
  const double n = bmin::parseDouble(b);
  setStorage(storage, a, formatNumber(n));
}

void StringEvaluatorFuncs::MOD_NUM(const String& a, const String& b) {
  if (!bmin::isDouble(b)) {
    throw std::runtime_error(("Invalid number value: " + b).cStr());
  }
  const double n = bmin::parseDouble(b);
  auto current = getStorage(storage, a);
  double currentN = 0.0;
  if (current) {
    if (!bmin::isDouble(*current)) {
      throw std::runtime_error(("Variable " + a + " is not a number").cStr());
    }
    currentN = bmin::parseDouble(*current);
  }
  setStorage(storage, a, formatNumber(currentN + n));
}

void StringEvaluatorFuncs::SET_STR(const String& a, const String& b) {
  setStorage(storage, a, b);
}

void StringEvaluatorFuncs::SETUP_DISPOSITION(const String& characterName) {
  // noop
}

void StringEvaluatorFuncs::START_QUEST(const String& questName) {
  // noop
}

void StringEvaluatorFuncs::COMPLETE_QUEST_STEP(const String& questName,
                                               const String& stepId) {
  // noop
}

void StringEvaluatorFuncs::COMPLETE_QUEST(const String& questName) {
  // noop
}

void StringEvaluatorFuncs::SPAWN_CH(const String& chName) {
  // noop
}

void StringEvaluatorFuncs::DESPAWN_CH(const String& chName) {
  // noop
}

void StringEvaluatorFuncs::CHANGE_TILE_AT(const String& x, const String& y,
                                          const String& tileName) {
  // noop
}

void StringEvaluatorFuncs::TELEPORT_TO(const String& x, const String& y,
                                       const String& mapName) {
  // noop
}

void StringEvaluatorFuncs::ADD_ITEM_AT(const String& x, const String& y,
                                       const String& itemName) {
  // noop
}

void StringEvaluatorFuncs::REMOVE_ITEM_AT(const String& x, const String& y,
                                          const String& itemName) {
  // noop
}

void StringEvaluatorFuncs::ADD_ITEM_TO_PLAYER(const String& itemName) {
  // noop
}

void StringEvaluatorFuncs::REMOVE_ITEM_FROM_PLAYER(const String& itemName) {
  // noop
}

void StringEvaluatorFuncs::OPEN_SHOP(const String& shopName) {
  // noop
}

StringEvaluator::StringEvaluator(bmin::Map<String, String>& storage,
                                 const String& baseStringStr)
    : baseStringStr(baseStringStr), funcs(storage) {}

void StringEvaluator::assertFuncArgs(const String& funcName,
                                     const DynArray<String>& funcArgs,
                                     size_t expectedArgs) {
  if (funcArgs.size() != expectedArgs) {
    throw std::runtime_error(
        ("Invalid number of arguments for function '" + funcName + "'. Expected " +
         bmin::toString(expectedArgs) + ", got " + bmin::toString(funcArgs.size()))
            .cStr());
  }
}

void StringEvaluator::evalStr(const String& str) {
  if (isFunctionCall(str)) {
    FunctionCall call = parseFunctionCall(str);
    if (call.funcName == "GET") {
      assertFuncArgs(call.funcName, call.args, 1);
      strResult = funcs.GET(call.args[0]);
    } else if (call.funcName == "SET_BOOL") {
      if (call.args.size() == 1) {
        funcs.SET_BOOL(call.args[0], "true");
      } else {
        assertFuncArgs(call.funcName, call.args, 2);
        funcs.SET_BOOL(call.args[0], call.args[1]);
      }
    } else if (call.funcName == "SET_NUM") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.SET_NUM(call.args[0], call.args[1]);
    } else if (call.funcName == "MOD_NUM") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.MOD_NUM(call.args[0], call.args[1]);
    } else if (call.funcName == "SET_STR") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.SET_STR(call.args[0], call.args[1]);
    } else if (call.funcName == "SETUP_DISPOSITION") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.SETUP_DISPOSITION(call.args[0]);
    } else if (call.funcName == "START_QUEST") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.START_QUEST(call.args[0]);
    } else if (call.funcName == "COMPLETE_QUEST_STEP") {
      assertFuncArgs(call.funcName, call.args, 2);
      funcs.COMPLETE_QUEST_STEP(call.args[0], call.args[1]);
    } else if (call.funcName == "COMPLETE_QUEST") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.COMPLETE_QUEST(call.args[0]);
    } else if (call.funcName == "SPAWN_CH") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.SPAWN_CH(call.args[0]);
    } else if (call.funcName == "DESPAWN_CH") {
      assertFuncArgs(call.funcName, call.args, 1);
      funcs.DESPAWN_CH(call.args[0]);
    } else if (call.funcName == "CHANGE_TILE_AT") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.CHANGE_TILE_AT(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "TELEPORT_TO") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.TELEPORT_TO(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "ADD_ITEM_AT") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.ADD_ITEM_AT(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "REMOVE_ITEM_AT") {
      assertFuncArgs(call.funcName, call.args, 3);
      funcs.REMOVE_ITEM_AT(call.args[0], call.args[1], call.args[2]);
    } else {
      throw std::runtime_error(
          ("Function '" + call.funcName + "' not found: " + baseStringStr).cStr());
    }
  } else {
    throw std::runtime_error(("Invalid eval string: " + baseStringStr).cStr());
  }
}

} // namespace runner
