#pragma once

#include "game/combat/CombatParty.h"
#include "game/map/MapVision.h"
#include "model/Combat.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "actions/combat/ActionBase.hpp"
#include "actions/combat/SetActiveCombatCharacter.hpp"

namespace state {

namespace actions {

class StartCombat : public CombatAction {
  ActionEvent getEvent() const override { return ActionEvent::StartCombat; }
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
    game::addPartyMembersToCombatMap(world, state->player, *database);
    game::updateActiveMapVisibilityFromParty(
        world, state->mapInstances, state->player, *database);
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
