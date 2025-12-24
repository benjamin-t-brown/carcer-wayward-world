#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace runner {

// Helper functions for storage (flat map, no nesting)
void setStorage(std::unordered_map<std::string, std::string>& storage,
                const std::string& key, const std::string& value);

std::optional<std::string>
getStorage(const std::unordered_map<std::string, std::string>& storage,
           const std::string& key);

// Helper to split string by delimiter
std::vector<std::string> splitString(const std::string& str, char delimiter);

// Helper to trim whitespace
std::string trim(const std::string& str);

// Parse function call: "FUNC_NAME(arg1, arg2, ...)"
struct FunctionCall {
  std::string funcName;
  std::vector<std::string> args;
};

FunctionCall parseFunctionCall(const std::string& str);

bool isFunctionCall(const std::string& str);

} // namespace runner

