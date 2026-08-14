#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "state/actions/combat/ModifyHP.hpp"
#include "state/actions/combat/PlaySound.hpp"
#include "state/actions/world/WorldSpawnDamageParticle.hpp"

namespace state {

namespace actions {

class PerformMeleeAttack : public CombatAction {
  bmin::String attackerId;
  bmin::String victimId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    auto* attacker = orch.findCharacterById(attackerId);
    auto* victim = orch.findCharacterById(victimId);
    if (attacker == nullptr || victim == nullptr) {
      return;
    }

    model::updateCharacterFacingToward(*attacker, victim->x, victim->y);

    insertAction(new CharacterSetSpriteIndexOffset(attackerId, 1), 0);

    const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
    if (hit) {
      insertAction(new PlaySound("punch1"), 0);
      insertAction(nullptr, 75);
      insertAction(new ModifyHP(victimId, -model::COMBAT_MELEE_DAMAGE), 0);
      insertAction(
          new WorldSpawnDamageParticle("splash_attack",
                                       bmin::toString(model::COMBAT_MELEE_DAMAGE),
                                       victim->x,
                                       victim->y,
                                       500),
          0);
      insertAction(nullptr, 500);
    } else {
      insertAction(new PlaySound("whip"), 0);
      insertAction(nullptr, 300);
    }
    insertAction(new CharacterSetSpriteIndexOffset(attackerId, 0), 0);
  }

public:
  PerformMeleeAttack(bmin::String _attackerId, bmin::String _victimId)
      : attackerId(std::move(_attackerId)), victimId(std::move(_victimId)) {}
};

} // namespace actions

} // namespace state
