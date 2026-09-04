module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.model.templates:Spells;
export import bmin.containers;
import bmin.string_interop;
export import :RuneTypes;

export {

// --- from model/templates/Spells.h ---
namespace model {

struct SpellRuneRequirement {
  RuneType type = RuneType::HEAT;
  int count = 1;
};

struct SpellTemplate {
  bmin::String name;
  bmin::String label;
  bmin::String description;
  bmin::String icon;
  bmin::String abilityName;
  bmin::DynArray<SpellRuneRequirement> requiredRunes;
};

} // namespace model

} // export
