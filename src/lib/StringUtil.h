#pragma once

#include "bmin/DynArray.h"  // IWYU pragma: keep
#include "bmin/String.h"      // IWYU pragma: keep

#include <string_view>

namespace strutil {

inline bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline size_t findFirstNotOf(const bmin::String& str, const char* chars) {
  for (size_t i = 0; i < str.size(); ++i) {
    bool match = false;
    for (const char* p = chars; *p != '\0'; ++p) {
      if (str[i] == *p) {
        match = true;
        break;
      }
    }
    if (!match) {
      return i;
    }
  }
  return bmin::String::npos;
}

inline size_t findLastNotOf(const bmin::String& str, const char* chars) {
  if (str.empty()) {
    return bmin::String::npos;
  }
  for (size_t i = str.size(); i > 0; --i) {
    bool match = false;
    for (const char* p = chars; *p != '\0'; ++p) {
      if (str[i - 1] == *p) {
        match = true;
        break;
      }
    }
    if (!match) {
      return i - 1;
    }
  }
  return bmin::String::npos;
}

inline bmin::String trim(const bmin::String& str) {
  constexpr const char* kWhitespace = " \t\n\r";
  const size_t first = findFirstNotOf(str, kWhitespace);
  if (first == bmin::String::npos) {
    return {};
  }
  const size_t last = findLastNotOf(str, kWhitespace);
  return str.substr(first, last - first + 1);
}

inline bmin::DynArray<bmin::String> splitByChar(const bmin::String& str, char delimiter) {
  bmin::DynArray<bmin::String> result;
  size_t start = 0;
  for (size_t i = 0; i <= str.size(); ++i) {
    if (i == str.size() || str[i] == delimiter) {
      result.pushBack(str.substr(start, i - start));
      start = i + 1;
    }
  }
  return result;
}

inline bmin::DynArray<bmin::String> splitLines(const bmin::String& str) {
  bmin::DynArray<bmin::String> lines = str.split("\n");
  for (size_t i = 0; i < lines.size(); ++i) {
    if (!lines[i].empty() && lines[i][lines[i].size() - 1] == '\r') {
      lines[i].erase(lines[i].size() - 1, 1);
    }
  }
  return lines;
}

// First whitespace-delimited token as name; remaining text parsed as int.
inline bool parseFirstTokenAndInt(const bmin::String& line, bmin::String& name, int& value) {
  size_t i = 0;
  while (i < line.size() && isWhitespace(line[i])) {
    ++i;
  }
  if (i >= line.size()) {
    return false;
  }
  const size_t nameStart = i;
  while (i < line.size() && !isWhitespace(line[i])) {
    ++i;
  }
  name = line.substr(nameStart, i - nameStart);
  while (i < line.size() && isWhitespace(line[i])) {
    ++i;
  }
  if (i >= line.size()) {
    return false;
  }
  const bmin::String remainder = line.substr(i);
  if (!bmin::isInt(remainder)) {
    return false;
  }
  value = bmin::parseInt(remainder);
  return true;
}

inline bmin::String fromStringView(std::string_view sv) {
  return bmin::String(sv.data(), sv.size());
}

}  // namespace strutil
