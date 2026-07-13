#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "lib/bmin/Map.h"

#include <optional>

namespace runner {

// Helper functions for storage (flat map, no nesting)
void setStorage(bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& key,
                const bmin::String& value);

std::optional<bmin::String> getStorage(const bmin::Map<bmin::String, bmin::String>& storage,
                                       const bmin::String& key);

// Drop per-conversation scratch keys (prefix "tmp."). Keeps once.tmp.* and vars.*.
void clearTmpStorageKeys(bmin::Map<bmin::String, bmin::String>& storage);

// Split exec/eval strings into statements (newlines or semicolons, not inside parens)
bmin::DynArray<bmin::String> splitExecStatements(const bmin::String& str);

// Helper to split string by delimiter
bmin::DynArray<bmin::String> splitString(const bmin::String& str, char delimiter);

// Helper to trim whitespace
bmin::String trim(const bmin::String& str);

// Parse function call: "FUNC_NAME(arg1, arg2, ...)"
struct FunctionCall {
  bmin::String funcName;
  bmin::DynArray<bmin::String> args;
};

FunctionCall parseFunctionCall(const bmin::String& str);

bool isFunctionCall(const bmin::String& str);

} // namespace runner
