#pragma once

#include "game/combat/EnemyBehavior.h"
#include "game/map/ActiveMapCharacters.h"
#include "model/Combat.h"
#include "model/templates/CharacterTemplate.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "actions/combat/DoCombatAction.hpp"

namespace state {

namespace actions {

class DoCPUCombatTurn : public AbstractAction {
  const int CPU_TURN_DELAY = 100;

  ActionEvent getEvent() const override { return ActionEvent::DoCPUCombatTurn; }
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

    auto* actor = game::findCharacterById(world.activeMap, actorId);
    if (actor == nullptr || model::isCharacterDefeated(state->player, *actor)) {
      insertAction(state::makeAction<DoCombatActionCompletion>(), 0);
      return;
    }

    if (actor->currentAp <= 0) {
      insertAction(nullptr, scaledDelayMs(CPU_TURN_DELAY));
      insertAction(state::makeAction<DoCombatAction>(actorId, model::CombatActionType::WAIT),
                   0);
      return;
    }

    const auto turnBehavior = game::resolveCombatTurnBehavior(*actor);
    if (turnBehavior == model::CombatBehaviorName::SPELLCASTER) {
      game::EnemySpellAction spellAction;
      if (game::chooseEnemySpellAction(
              world, state->mapInstances, state->player, *actor, *database, spellAction)) {
        insertAction(nullptr, scaledDelayMs(CPU_TURN_DELAY));
        insertAction(state::makeAction<DoCombatAction>(
                         actorId,
                         model::CombatActionType::SPELL,
                         CombatActionContext{.targetChId = spellAction.targetChId,
                                             .abilityId = spellAction.spellName,
                                             .targetLoc = spellAction.targetLoc}),
                     0);
        return;
      }
    }

    if (turnBehavior == model::CombatBehaviorName::SEEK_AND_MELEE ||
        turnBehavior == model::CombatBehaviorName::SPELLCASTER) {
      auto dx = 0;
      auto dy = 0;
      if (game::chooseSeekAndMeleeCombatAction(
              world, state->mapInstances, state->player, *actor, *database, dx, dy)) {
        insertAction(nullptr, scaledDelayMs(CPU_TURN_DELAY));
        insertAction(state::makeAction<DoCombatAction>(
                         actorId, model::CombatActionType::MOVE, CombatActionContext{.targetLoc = {dx, dy}}),
                     0);
        return;
      }
    }

    insertAction(nullptr, scaledDelayMs(CPU_TURN_DELAY));
    insertAction(state::makeAction<DoCombatAction>(actorId, model::CombatActionType::WAIT), 0);
  }

public:
  DoCPUCombatTurn() = default;
};

} // namespace actions

} // namespace state
