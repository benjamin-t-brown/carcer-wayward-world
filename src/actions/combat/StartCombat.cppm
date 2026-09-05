module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:StartCombat;
export import carcer.state;
import :SetActiveCombatCharacter;
import carcer.game.map;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class StartCombat : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    LOG(INFO) << "StartCombat: starting combat on grid " << world.activeMap.gridId
              << LOG_ENDL;
    state->turnMode = model::TurnMode::TURN_COMBAT;
    model::addPartyMembersToCombatMap(world, state->player, *database);
    game::updateActiveMapVisibilityFromParty(world, state->player, *database);
    world.combat = model::createCombatFromWorld(world, state->player);
    model::resetAllCombatAp(world, model::COMBAT_STARTING_AP);

    if (world.combat.turnOrderIds.empty()) {
      world.combat.active = false;
      LOG(WARN) << "StartCombat: no combatants found, aborting" << LOG_ENDL;
      return;
    }

    world.combat.activeTurnIndex = 0;
    LOG(INFO) << "StartCombat: turn order has " << world.combat.turnOrderIds.size()
              << " characters" << LOG_ENDL;
    insertAction(new SetActiveCombatCharacter(), 0);
  }

public:
  StartCombat() = default;
};

} // namespace actions

} // namespace state

} // export
