#include "EventRunnerHelpers.h"
#include "lib/StringUtil.h"

namespace runner {

// Helper functions for storage (flat map, no nesting)
void setStorage(bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& key,
                const bmin::String& value) {
  storage[key] = value;
}

std::optional<bmin::String> getStorage(const bmin::Map<bmin::String, bmin::String>& storage,
                                 const bmin::String& key) {
  auto it = const_cast<bmin::Map<bmin::String, bmin::String>&>(storage).find(key);
  if (it != storage.end()) {
    return it->value;
  }
  return std::nullopt;
}

void clearTmpStorageKeys(bmin::Map<bmin::String, bmin::String>& storage) {
  bmin::DynArray<bmin::String> keysToErase;
  for (auto it = storage.begin(); it != storage.end(); ++it) {
    if (it->key.startsWith("tmp.")) {
      keysToErase.pushBack(it->key);
    }
  }
  for (const auto& key : keysToErase) {
    storage.erase(key);
  }
}

// Helper to trim whitespace
bmin::String trim(const bmin::String& str) {
  return strutil::trim(str);
}

// Helper to split string by delimiter
bmin::DynArray<bmin::String> splitString(const bmin::String& str, char delimiter) {
  const bmin::DynArray<bmin::String> parts = strutil::splitByChar(str, delimiter);
  bmin::DynArray<bmin::String> result;
  result.reserve(parts.size());
  for (size_t i = 0; i < parts.size(); ++i) {
    result.pushBack(parts[i]);
  }
  return result;
}

bmin::DynArray<bmin::String> splitExecStatements(const bmin::String& str) {
  bmin::DynArray<bmin::String> result;
  int parenDepth = 0;
  bmin::String current;
  for (size_t i = 0; i < str.size(); ++i) {
    const char c = str[i];
    if (c == '(') {
      parenDepth++;
      current += c;
    } else if (c == ')') {
      parenDepth--;
      current += c;
    } else if ((c == ';' || c == '\n') && parenDepth == 0) {
      const bmin::String trimmed = trim(current);
      if (!trimmed.empty()) {
        result.pushBack(trimmed);
      }
      current.clear();
    } else {
      current += c;
    }
  }
  const bmin::String trimmed = trim(current);
  if (!trimmed.empty()) {
    result.pushBack(trimmed);
  }
  return result;
}

// Parse function call: "FUNC_NAME(arg1, arg2, ...)"
FunctionCall parseFunctionCall(const bmin::String& str) {
  FunctionCall result;
  const size_t openParen = str.find("(");
  if (openParen == bmin::String::npos) {
    result.funcName = trim(str);
    return result;
  }

  result.funcName = trim(str.substr(0, openParen));
  size_t closeParen = bmin::String::npos;
  for (size_t i = str.size(); i > 0; --i) {
    if (str[i - 1] == ')') {
      closeParen = i - 1;
      break;
    }
  }
  if (closeParen == bmin::String::npos || closeParen <= openParen) {
    return result;
  }

  const bmin::String argsStr = str.substr(openParen + 1, closeParen - openParen - 1);

  // Properly parse comma-separated args allowing for nested parentheses
  size_t parenDepth = 0;
  bmin::String currentArg;
  for (size_t i = 0; i < argsStr.size(); ++i) {
    const char c = argsStr[i];
    if (c == '(') {
      parenDepth++;
      currentArg += c;
    } else if (c == ')') {
      parenDepth--;
      currentArg += c;
    } else if (c == ',' && parenDepth == 0) {
      result.args.pushBack(trim(currentArg));
      currentArg.clear();
    } else {
      currentArg += c;
    }
  }
  if (!currentArg.empty()) {
    result.args.pushBack(trim(currentArg));
  }

  return result;
}

bool isFunctionCall(const bmin::String& str) {
  return str.find("(") != bmin::String::npos && str.find(")") != bmin::String::npos;
}

} // namespace runner
