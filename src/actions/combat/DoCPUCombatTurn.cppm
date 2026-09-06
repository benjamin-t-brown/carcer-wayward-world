module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:DoCPUCombatTurn;
export import carcer.state;
import :DoCombatAction;
import carcer.game.map;
import carcer.game.combat;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class DoCPUCombatTurn : public AbstractAction {
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

    game::ActiveMapOrchestrator orch(
        world.activeMap, state->mapInstances, getDatabase());
    if (!world.activeMap.gridId.empty()) {
      orch.fetchMapGrid(world.activeMap.gridId);
    }
    auto* actor = orch.findCharacterById(actorId);
    if (actor == nullptr) {
      insertAction(nullptr, 300);
      insertAction(new DoCombatAction(actorId, model::CombatActionType::WAIT), 0);
      return;
    }

    if (actor->combatBehaviorCombat == model::CombatBehaviorName::SEEK_AND_MELEE) {
      auto dx = 0;
      auto dy = 0;
      if (game::chooseSeekAndMeleeCombatAction(
              world, state->mapInstances, state->player, *actor, *database, dx, dy)) {
        insertAction(nullptr, 300);
        insertAction(new DoCombatAction(
                         actorId, model::CombatActionType::MOVE, {.targetLoc = {dx, dy}}),
                     0);
        return;
      }
    }

    insertAction(nullptr, 300);
    insertAction(new DoCombatAction(actorId, model::CombatActionType::WAIT), 0);
  }

public:
  DoCPUCombatTurn() = default;
};

} // namespace actions

} // namespace state

} // export
