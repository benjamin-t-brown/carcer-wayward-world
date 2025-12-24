#include "StringEvaluator.h"
#include <stdexcept>

namespace runner {

StringEvaluatorFuncs::StringEvaluatorFuncs(
    std::unordered_map<std::string, std::string>& storage)
    : storage(storage) {}

std::string StringEvaluatorFuncs::GET(const std::string& a) {
  auto v = getStorage(storage, a);
  return v.value_or("");
}

void StringEvaluatorFuncs::SET_BOOL(const std::string& a, const std::string& b) {
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

void StringEvaluatorFuncs::SET_NUM(const std::string& a, const std::string& b) {
  try {
    double n = std::stod(b);
    setStorage(storage, a, std::to_string(n));
  } catch (...) {
    throw std::runtime_error("Invalid number value: " + b);
  }
}

void StringEvaluatorFuncs::MOD_NUM(const std::string& a, const std::string& b) {
  try {
    double n = std::stod(b);
    auto current = getStorage(storage, a);
    double currentN = 0.0;
    if (current) {
      try {
        currentN = std::stod(*current);
      } catch (...) {
        throw std::runtime_error("Variable " + a + " is not a number");
      }
    }
    setStorage(storage, a, std::to_string(currentN + n));
  } catch (...) {
    throw std::runtime_error("Invalid number value: " + b);
  }
}

void StringEvaluatorFuncs::SET_STR(const std::string& a, const std::string& b) {
  setStorage(storage, a, b);
}

void StringEvaluatorFuncs::SETUP_DISPOSITION(const std::string& characterName) {
  // noop
}

void StringEvaluatorFuncs::START_QUEST(const std::string& questName) {
  // noop
}

void StringEvaluatorFuncs::ADVANCE_QUEST(const std::string& questName,
                                         const std::string& stepId) {
  // noop
}

void StringEvaluatorFuncs::COMPLETE_QUEST(const std::string& questName) {
  // noop
}

void StringEvaluatorFuncs::SPAWN_CH(const std::string& chName) {
  // noop
}

void StringEvaluatorFuncs::DESPAWN_CH(const std::string& chName) {
  // noop
}

void StringEvaluatorFuncs::CHANGE_TILE_AT(const std::string& x,
                                          const std::string& y,
                                          const std::string& tileName) {
  // noop
}

void StringEvaluatorFuncs::TELEPORT_TO(const std::string& x,
                                       const std::string& y,
                                       const std::string& mapName) {
  // noop
}

void StringEvaluatorFuncs::ADD_ITEM_AT(const std::string& x,
                                       const std::string& y,
                                       const std::string& itemName) {
  // noop
}

void StringEvaluatorFuncs::REMOVE_ITEM_AT(const std::string& x,
                                          const std::string& y,
                                          const std::string& itemName) {
  // noop
}

StringEvaluator::StringEvaluator(std::unordered_map<std::string, std::string>& storage,
                                 const std::string& baseStringStr)
    : baseStringStr(baseStringStr), funcs(storage) {}

void StringEvaluator::evalStr(const std::string& str) {
  if (isFunctionCall(str)) {
    FunctionCall call = parseFunctionCall(str);
    if (call.funcName == "GET") {
      if (call.args.size() != 1) {
        throw std::runtime_error("GET requires 1 argument");
      }
      funcs.GET(call.args[0]);
    } else if (call.funcName == "SET_BOOL") {
      if (call.args.size() != 2) {
        throw std::runtime_error("SET_BOOL requires 2 arguments");
      }
      funcs.SET_BOOL(call.args[0], call.args[1]);
    } else if (call.funcName == "SET_NUM") {
      if (call.args.size() != 2) {
        throw std::runtime_error("SET_NUM requires 2 arguments");
      }
      funcs.SET_NUM(call.args[0], call.args[1]);
    } else if (call.funcName == "MOD_NUM") {
      if (call.args.size() != 2) {
        throw std::runtime_error("MOD_NUM requires 2 arguments");
      }
      funcs.MOD_NUM(call.args[0], call.args[1]);
    } else if (call.funcName == "SET_STR") {
      if (call.args.size() != 2) {
        throw std::runtime_error("SET_STR requires 2 arguments");
      }
      funcs.SET_STR(call.args[0], call.args[1]);
    } else if (call.funcName == "SETUP_DISPOSITION") {
      if (call.args.size() != 1) {
        throw std::runtime_error("SETUP_DISPOSITION requires 1 argument");
      }
      funcs.SETUP_DISPOSITION(call.args[0]);
    } else if (call.funcName == "START_QUEST") {
      if (call.args.size() != 1) {
        throw std::runtime_error("START_QUEST requires 1 argument");
      }
      funcs.START_QUEST(call.args[0]);
    } else if (call.funcName == "ADVANCE_QUEST") {
      if (call.args.size() != 2) {
        throw std::runtime_error("ADVANCE_QUEST requires 2 arguments");
      }
      funcs.ADVANCE_QUEST(call.args[0], call.args[1]);
    } else if (call.funcName == "COMPLETE_QUEST") {
      if (call.args.size() != 1) {
        throw std::runtime_error("COMPLETE_QUEST requires 1 argument");
      }
      funcs.COMPLETE_QUEST(call.args[0]);
    } else if (call.funcName == "SPAWN_CH") {
      if (call.args.size() != 1) {
        throw std::runtime_error("SPAWN_CH requires 1 argument");
      }
      funcs.SPAWN_CH(call.args[0]);
    } else if (call.funcName == "DESPAWN_CH") {
      if (call.args.size() != 1) {
        throw std::runtime_error("DESPAWN_CH requires 1 argument");
      }
      funcs.DESPAWN_CH(call.args[0]);
    } else if (call.funcName == "CHANGE_TILE_AT") {
      if (call.args.size() != 3) {
        throw std::runtime_error("CHANGE_TILE_AT requires 3 arguments");
      }
      funcs.CHANGE_TILE_AT(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "TELEPORT_TO") {
      if (call.args.size() != 3) {
        throw std::runtime_error("TELEPORT_TO requires 3 arguments");
      }
      funcs.TELEPORT_TO(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "ADD_ITEM_AT") {
      if (call.args.size() != 3) {
        throw std::runtime_error("ADD_ITEM_AT requires 3 arguments");
      }
      funcs.ADD_ITEM_AT(call.args[0], call.args[1], call.args[2]);
    } else if (call.funcName == "REMOVE_ITEM_AT") {
      if (call.args.size() != 3) {
        throw std::runtime_error("REMOVE_ITEM_AT requires 3 arguments");
      }
      funcs.REMOVE_ITEM_AT(call.args[0], call.args[1], call.args[2]);
    } else {
      throw std::runtime_error("Function '" + call.funcName +
                               "' not found: " + baseStringStr);
    }
  } else {
    throw std::runtime_error("Invalid eval string: " + baseStringStr);
  }
}

} // namespace runner
