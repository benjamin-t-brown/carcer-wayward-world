#pragma once

#include "model/Combat.h"
#include "game/map/MapVision.h"
#include "model/instances/World.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/SetActiveCombatCharacter.hpp"

namespace state {

namespace actions {

class StartCombat : public CombatAction {
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    LOG(INFO) << "StartCombat: starting combat on map " << world.currentMap.id << LOG_ENDL;
    world.currentMap.turnMode = model::TurnMode::TURN_COMBAT;
    model::addPartyMembersToCombatMap(world, state->player, *database);
    game::updateMapVisibilityFromParty(world.currentMap, state->player, *database);
    world.combat = model::createCombatFromWorld(world, state->player, *database);
    model::resetAllCombatAp(world, model::COMBAT_STARTING_AP);

    if (world.combat.turnOrderIds.empty()) {
      world.combat.active = false;
      LOG(WARN) << "StartCombat: no combatants found, aborting" << LOG_ENDL;
      return;
    }

    world.combat.activeTurnIndex = 0;
    LOG(INFO) << "StartCombat: turn order has " << world.combat.turnOrderIds.size()
              << " characters" << LOG_ENDL;
    insertCombatAction(new SetActiveCombatCharacter(), 0);
  }

public:
  StartCombat() = default;
};

} // namespace actions

} // namespace state
