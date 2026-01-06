#include "EventRunnerHelpers.h"
#include <sstream>

namespace runner {

// Helper functions for storage (flat map, no nesting)
void setStorage(std::unordered_map<std::string, std::string>& storage,
                const std::string& key, const std::string& value) {
  storage[key] = value;
}

std::optional<std::string>
getStorage(const std::unordered_map<std::string, std::string>& storage,
           const std::string& key) {
  auto it = storage.find(key);
  if (it != storage.end()) {
    return it->second;
  }
  return std::nullopt;
}

// Helper to split string by delimiter
std::vector<std::string> splitString(const std::string& str, char delimiter) {
  std::vector<std::string> result;
  std::stringstream ss(str);
  std::string item;
  while (std::getline(ss, item, delimiter)) {
    result.push_back(item);
  }
  return result;
}

// Helper to trim whitespace
std::string trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, (last - first + 1));
}

// Parse function call: "FUNC_NAME(arg1, arg2, ...)"
FunctionCall parseFunctionCall(const std::string& str) {
  FunctionCall result;
  size_t openParen = str.find('(');
  if (openParen == std::string::npos) {
    result.funcName = trim(str);
    return result;
  }

  result.funcName = trim(str.substr(0, openParen));
  size_t closeParen = str.rfind(')');
  if (closeParen == std::string::npos || closeParen <= openParen) {
    return result;
  }

  std::string argsStr = str.substr(openParen + 1, closeParen - openParen - 1);
  std::vector<std::string> args;

  // Properly parse comma-separated args allowing for nested parentheses
  size_t parenDepth = 0;
  std::string currentArg;
  for (size_t i = 0; i < argsStr.length(); ++i) {
    char c = argsStr[i];
    if (c == '(') {
      parenDepth++;
      currentArg += c;
    } else if (c == ')') {
      parenDepth--;
      currentArg += c;
    } else if (c == ',' && parenDepth == 0) {
      result.args.push_back(trim(currentArg));
      currentArg.clear();
    } else {
      currentArg += c;
    }
  }
  if (!currentArg.empty()) {
    result.args.push_back(trim(currentArg));
  }

  return result;
}

bool isFunctionCall(const std::string& str) {
  return str.find('(') != std::string::npos && str.find(')') != std::string::npos;
}

} // namespace runner

