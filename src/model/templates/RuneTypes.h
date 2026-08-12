#pragma once

#include "bmin/String.h"

namespace model {

enum class RuneType {
  HEAT = 0,
  ENTROPY,
  REGROWTH,
  DISPLACE,
  EXPAND,
  ATTACH,
  TRANSFORM,
  COMPACT,
};

inline constexpr int kRuneTypeCount = 8;

RuneType runeTypeFromString(const bmin::String& value);
bmin::String runeTypeToString(RuneType value);

int runeTypeIndex(RuneType value);
RuneType runeTypeFromIndex(int index);

bmin::String runeTypeToSpriteName(RuneType value);

} // namespace model
