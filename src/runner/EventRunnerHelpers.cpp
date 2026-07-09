#include "EventRunnerHelpers.h"
#include "lib/StringUtil.h"

namespace runner {

// Helper functions for storage (flat map, no nesting)
void setStorage(bmin::Map<String, String>& storage, const String& key,
                const String& value) {
  storage[key] = value;
}

std::optional<String> getStorage(const bmin::Map<String, String>& storage,
                                 const String& key) {
  auto it = const_cast<bmin::Map<String, String>&>(storage).find(key);
  if (it != storage.end()) {
    return it->value;
  }
  return std::nullopt;
}

// Helper to trim whitespace
String trim(const String& str) {
  return strutil::trim(str);
}

// Helper to split string by delimiter
DynArray<String> splitString(const String& str, char delimiter) {
  const bmin::DynArray<String> parts = strutil::splitByChar(str, delimiter);
  DynArray<String> result;
  result.reserve(parts.size());
  for (size_t i = 0; i < parts.size(); ++i) {
    result.pushBack(parts[i]);
  }
  return result;
}

DynArray<String> splitExecStatements(const String& str) {
  DynArray<String> result;
  int parenDepth = 0;
  String current;
  for (size_t i = 0; i < str.size(); ++i) {
    const char c = str[i];
    if (c == '(') {
      parenDepth++;
      current += c;
    } else if (c == ')') {
      parenDepth--;
      current += c;
    } else if ((c == ';' || c == '\n') && parenDepth == 0) {
      const String trimmed = trim(current);
      if (!trimmed.empty()) {
        result.pushBack(trimmed);
      }
      current.clear();
    } else {
      current += c;
    }
  }
  const String trimmed = trim(current);
  if (!trimmed.empty()) {
    result.pushBack(trimmed);
  }
  return result;
}

// Parse function call: "FUNC_NAME(arg1, arg2, ...)"
FunctionCall parseFunctionCall(const String& str) {
  FunctionCall result;
  const size_t openParen = str.find("(");
  if (openParen == String::npos) {
    result.funcName = trim(str);
    return result;
  }

  result.funcName = trim(str.substr(0, openParen));
  size_t closeParen = String::npos;
  for (size_t i = str.size(); i > 0; --i) {
    if (str[i - 1] == ')') {
      closeParen = i - 1;
      break;
    }
  }
  if (closeParen == String::npos || closeParen <= openParen) {
    return result;
  }

  const String argsStr = str.substr(openParen + 1, closeParen - openParen - 1);

  // Properly parse comma-separated args allowing for nested parentheses
  size_t parenDepth = 0;
  String currentArg;
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

bool isFunctionCall(const String& str) {
  return str.find("(") != String::npos && str.find(")") != String::npos;
}

} // namespace runner
