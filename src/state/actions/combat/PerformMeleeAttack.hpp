#pragma once

#include "model/Combat.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/ModifyHP.hpp"
#include "state/actions/combat/PlaySound.hpp"
#include "state/actions/ui/UiPushFloatingNotification.hpp"

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

    auto& map = state->world.currentMap;
    auto* attacker = model::findCharacterOnMap(map, attackerId);
    auto* victim = model::findCharacterOnMap(map, victimId);
    if (attacker == nullptr || victim == nullptr) {
      return;
    }

    attacker->spriteIndex += 1;

    const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
    if (hit) {
      insertCombatAction(new PlaySound("punch1"), 0);
      insertCombatAction(new ModifyHP(victimId, -model::COMBAT_MELEE_DAMAGE), 0);
      auto damageText = bmin::String("-") + bmin::toString(model::COMBAT_MELEE_DAMAGE);
      insertCombatAction(new UiPushFloatingNotification(std::move(damageText),
                                                        UiFloatingNotificationType::INFO),
                         0);
      insertCombatAction(nullptr, 500);
    } else {
      insertCombatAction(new PlaySound("whip"), 0);
      insertCombatAction(nullptr, 300);
    }
  }

public:
  PerformMeleeAttack(bmin::String _attackerId, bmin::String _victimId)
      : attackerId(std::move(_attackerId)), victimId(std::move(_victimId)) {}
};

} // namespace actions

} // namespace state
