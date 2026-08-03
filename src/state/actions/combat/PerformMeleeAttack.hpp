#pragma once

#include "model/instances/CharacterInstance.h"
#include "model/Combat.h"
#include "game/map/ActiveMapOrchestrator.h"
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

    insertCombatAction(new CharacterSetSpriteIndexOffset(attackerId, 1), 0);

    const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
    if (hit) {
      insertCombatAction(new PlaySound("punch1"), 0);
      insertCombatAction(nullptr, 75);
      insertCombatAction(new ModifyHP(victimId, -model::COMBAT_MELEE_DAMAGE), 0);
      insertCombatAction(new WorldSpawnDamageParticle("splash_attack",
                                                      victim->x,
                                                      victim->y,
                                                      model::COMBAT_MELEE_DAMAGE,
                                                      500),
                         0);
      insertCombatAction(nullptr, 500);
    } else {
      insertCombatAction(new PlaySound("whip"), 0);
      insertCombatAction(nullptr, 300);
    }
    insertCombatAction(new CharacterSetSpriteIndexOffset(attackerId, 0), 0);
  }

public:
  PerformMeleeAttack(bmin::String _attackerId, bmin::String _victimId)
      : attackerId(std::move(_attackerId)), victimId(std::move(_victimId)) {}
};

} // namespace actions

} // namespace state
