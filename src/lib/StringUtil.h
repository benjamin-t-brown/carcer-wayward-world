#pragma once

#include "Types.h"

#include <string_view>

namespace strutil {

inline bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline size_t findFirstNotOf(const String& str, const char* chars) {
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
  return String::npos;
}

inline size_t findLastNotOf(const String& str, const char* chars) {
  if (str.empty()) {
    return String::npos;
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
  return String::npos;
}

inline String trim(const String& str) {
  constexpr const char* kWhitespace = " \t\n\r";
  const size_t first = findFirstNotOf(str, kWhitespace);
  if (first == String::npos) {
    return {};
  }
  const size_t last = findLastNotOf(str, kWhitespace);
  return str.substr(first, last - first + 1);
}

inline bmin::DynArray<String> splitByChar(const String& str, char delimiter) {
  bmin::DynArray<String> result;
  size_t start = 0;
  for (size_t i = 0; i <= str.size(); ++i) {
    if (i == str.size() || str[i] == delimiter) {
      result.pushBack(str.substr(start, i - start));
      start = i + 1;
    }
  }
  return result;
}

inline bmin::DynArray<String> splitLines(const String& str) {
  bmin::DynArray<String> lines = str.split("\n");
  for (size_t i = 0; i < lines.size(); ++i) {
    if (!lines[i].empty() && lines[i][lines[i].size() - 1] == '\r') {
      lines[i].erase(lines[i].size() - 1, 1);
    }
  }
  return lines;
}

// First whitespace-delimited token as name; remaining text parsed as int.
inline bool parseFirstTokenAndInt(const String& line, String& name, int& value) {
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
  const String remainder = line.substr(i);
  if (!bmin::isInt(remainder)) {
    return false;
  }
  value = bmin::parseInt(remainder);
  return true;
}

inline String fromStringView(std::string_view sv) {
  return stringFromView(sv);
}

}  // namespace strutil
