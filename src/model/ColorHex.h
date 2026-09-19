#pragma once

#include "bmin/String.h"
#include "bmin/StringInterop.h"

namespace model {

struct RgbColor {
  unsigned char r = 255;
  unsigned char g = 255;
  unsigned char b = 255;
};

inline int hexNibble(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

inline int hexByte(char hi, char lo) {
  const auto h = hexNibble(hi);
  const auto l = hexNibble(lo);
  if (h < 0 || l < 0) {
    return -1;
  }
  return (h << 4) | l;
}

/** Parses `#RGB` / `#RRGGBB` (optional `#`). Invalid input returns white. */
inline RgbColor rgbColorFromHex(const bmin::String& hex) {
  auto view = bmin::toStringView(hex);
  const auto* data = view.data();
  auto size = view.size();
  if (size > 0 && data[0] == '#') {
    data += 1;
    size -= 1;
  }

  RgbColor color;
  if (size == 3) {
    const auto r = hexByte(data[0], data[0]);
    const auto g = hexByte(data[1], data[1]);
    const auto b = hexByte(data[2], data[2]);
    if (r < 0 || g < 0 || b < 0) {
      return color;
    }
    color.r = static_cast<unsigned char>(r);
    color.g = static_cast<unsigned char>(g);
    color.b = static_cast<unsigned char>(b);
    return color;
  }
  if (size == 6) {
    const auto r = hexByte(data[0], data[1]);
    const auto g = hexByte(data[2], data[3]);
    const auto b = hexByte(data[4], data[5]);
    if (r < 0 || g < 0 || b < 0) {
      return color;
    }
    color.r = static_cast<unsigned char>(r);
    color.g = static_cast<unsigned char>(g);
    color.b = static_cast<unsigned char>(b);
  }
  return color;
}

} // namespace model
