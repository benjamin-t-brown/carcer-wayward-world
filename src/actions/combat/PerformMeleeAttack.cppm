module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:PerformMeleeAttack;
export import carcer.state;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

class PerformMeleeAttack : public AbstractAction {
  bmin::String attackerId;
  bmin::String victimId;

  // body in combat.cpp: needs carcer.actions.world (WorldSpawnDamageParticle),
  // impl-only - a genuine deferred/timed follow-up, not eliminable by inlining.
  void act() override;

public:
  PerformMeleeAttack(bmin::String _attackerId, bmin::String _victimId)
      : attackerId(std::move(_attackerId)), victimId(std::move(_victimId)) {}
};

} // namespace actions

} // namespace state

} // export
