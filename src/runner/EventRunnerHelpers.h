#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"

#include <optional>

namespace runner {

// Helper functions for storage (flat map, no nesting)
void setStorage(bmin::Map<String, String>& storage, const String& key,
                const String& value);

std::optional<String> getStorage(const bmin::Map<String, String>& storage,
                                 const String& key);

// Split exec/eval strings into statements (newlines or semicolons, not inside parens)
DynArray<String> splitExecStatements(const String& str);

// Helper to split string by delimiter
DynArray<String> splitString(const String& str, char delimiter);

// Helper to trim whitespace
String trim(const String& str);

// Parse function call: "FUNC_NAME(arg1, arg2, ...)"
struct FunctionCall {
  String funcName;
  DynArray<String> args;
};

FunctionCall parseFunctionCall(const String& str);

bool isFunctionCall(const String& str);

} // namespace runner
