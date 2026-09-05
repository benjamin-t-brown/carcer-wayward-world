module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:PerformSpellCast;
export import carcer.state;
import carcer.game.map;
import carcer.model.templates;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

class PerformSpellCast : public AbstractAction {
  bmin::String casterId;
  bmin::String spellId;
  model::SpellTargetInfo spellTargetInfo;

  // body in combat.cpp: needs carcer.actions.world (WorldSpawnProjectile /
  // WorldSpawnDamageParticle / WorldSetActionMode), impl-only - genuine
  // deferred/timed follow-ups, not eliminable by inlining.
  void doZoneSpell(const model::AbilityTemplate& ability,
                   model::CharacterInstance& caster,
                   game::ActiveMapOrchestrator& orch);

  bool verifySpellCanBeCast(const model::CharacterInstance& caster,
                            const model::AbilityTemplate& ability) {
    if (ability.costType == model::AbilityCostType::ABILITY_COST_MANA) {
      if (caster.currentMp < ability.costValue) {
        return false;
      }
    }

    return true;
  }

  void act() override;

public:
  explicit PerformSpellCast(const bmin::String& casterId,
                            const bmin::String& spellId,
                            const model::SpellTargetInfo& spellTargetInfo)
      : casterId(casterId), spellId(spellId), spellTargetInfo(spellTargetInfo) {}
};

} // namespace actions

} // namespace state

} // export
