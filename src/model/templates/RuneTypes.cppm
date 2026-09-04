module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <stdexcept>

export module carcer.model.templates.RuneTypes;
export import bmin.containers;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

// --- from model/templates/RuneTypes.h ---
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

} // export

namespace model {

RuneType runeTypeFromString(const bmin::String& value) {
  if (value == "HEAT") {
    return RuneType::HEAT;
  }
  if (value == "ENTROPY") {
    return RuneType::ENTROPY;
  }
  if (value == "REGROWTH") {
    return RuneType::REGROWTH;
  }
  if (value == "DISPLACE") {
    return RuneType::DISPLACE;
  }
  if (value == "EXPAND") {
    return RuneType::EXPAND;
  }
  if (value == "ATTACH") {
    return RuneType::ATTACH;
  }
  if (value == "TRANSFORM") {
    return RuneType::TRANSFORM;
  }
  if (value == "COMPACT") {
    return RuneType::COMPACT;
  }
  throw std::runtime_error(("Invalid RuneType: " + value).cStr());
}

bmin::String runeTypeToString(RuneType value) {
  switch (value) {
  case RuneType::HEAT:
    return "HEAT";
  case RuneType::ENTROPY:
    return "ENTROPY";
  case RuneType::REGROWTH:
    return "REGROWTH";
  case RuneType::DISPLACE:
    return "DISPLACE";
  case RuneType::EXPAND:
    return "EXPAND";
  case RuneType::ATTACH:
    return "ATTACH";
  case RuneType::TRANSFORM:
    return "TRANSFORM";
  case RuneType::COMPACT:
    return "COMPACT";
  }
  throw std::runtime_error("Unknown RuneType");
}

int runeTypeIndex(RuneType value) {
  const auto index = static_cast<int>(value);
  if (index < 0 || index >= kRuneTypeCount) {
    throw std::runtime_error("Unknown RuneType");
  }
  return index;
}

RuneType runeTypeFromIndex(int index) {
  if (index < 0 || index >= kRuneTypeCount) {
    throw std::runtime_error(
        ("Invalid RuneType index: " + bmin::toString(index)).cStr());
  }
  return static_cast<RuneType>(index);
}

bmin::String runeTypeToSpriteName(RuneType value) {
  return "runes_" + bmin::toString(runeTypeIndex(value));
}

} // namespace model
