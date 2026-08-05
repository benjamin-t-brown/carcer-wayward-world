#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/EnemyBehavior.h"
#include "model/Combat.h"
#include "model/templates/CharacterTemplate.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/DoCombatAction.hpp"

namespace state {

namespace actions {

class DoCPUCombatTurn : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    const auto& actorId = world.combat.activeCharacterId;

    // Stale CPU turns can land after GoNextCombatTurn advances to a party member.
    // Re-arm waiting so the player can act; do not auto-WAIT away their turn.
    if (model::isPartyMember(state->player, actorId)) {
      world.combat.isWaitingForAction = true;
      return;
    }

    LOG(INFO) << "DoCPUCombatTurn: choosing action for "
              << model::formatCharacterLogLabel(world.activeMap, actorId) << LOG_ENDL;

    game::ActiveMapOrchestrator orch;
    if (!world.activeMap.gridId.empty()) {
      orch.fetchMapGrid(world.activeMap.gridId);
    }
    auto* actor = orch.findCharacterById(actorId);
    if (actor == nullptr) {
      insertCombatAction(nullptr, 300);
      insertCombatAction(new DoCombatAction(model::CombatActionType::WAIT), 0);
      return;
    }

    if (actor->combatBehaviorCombat == model::CombatBehaviorName::SEEK_AND_MELEE) {
      auto dx = 0;
      auto dy = 0;
      if (game::chooseSeekAndMeleeCombatAction(
              world, state->player, *actor, *database, dx, dy)) {
        insertCombatAction(nullptr, 300);
        insertCombatAction(new DoCombatAction(model::CombatActionType::MOVE, dx, dy), 0);
        return;
      }
    }

    insertCombatAction(nullptr, 300);
    insertCombatAction(new DoCombatAction(model::CombatActionType::WAIT), 0);
  }

public:
  DoCPUCombatTurn() = default;
};

} // namespace actions

} // namespace state
