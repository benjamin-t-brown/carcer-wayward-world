#pragma once

#include "game/combat/EnemyBehavior.h"
#include "model/templates/CharacterTemplate.h"
#include "actions/combat/ActionBase.hpp"
#include "actions/world/ClearTownEnemyAiResolving.hpp"
#include "actions/world/TownEnemySeekAndMelee.hpp"

namespace state {

namespace actions {

// Spotting + one town action per agitated enemy (queued with combat-style delays).
class TownEnemyAiAfterPlayerMove : public CombatAction {
  ActionEvent getEvent() const override { return ActionEvent::TownEnemyAiAfterPlayerMove; }
  void act() override {
    if (!state) {
      return;
    }
    auto& world = state->world;
    if (world.combat.active) {
      world.resolvingTownEnemyAi = false;
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      world.resolvingTownEnemyAi = false;
      return;
    }

    world.resolvingTownEnemyAi = true;
    // Held-move stays active; LayerWorld pauses repeats while this flag is set.

    game::updateEnemySpotting(
        world, state->mapInstances, state->player, *database);

    for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
      const auto& character = world.activeMap.characters[i];
      if (!character.agitated) {
        continue;
      }
      if (!model::characterInstanceIsEnemy(character)) {
        continue;
      }
      if (character.behaviorName != "IMMOBILE_UNTIL_ENEMY_SPOTTED") {
        continue;
      }
      if (character.combatBehaviorTown != model::CombatBehaviorName::SEEK_AND_MELEE) {
        continue;
      }
      insertAction(new TownEnemySeekAndMelee(character.id), 0);
    }

    insertAction(new ClearTownEnemyAiResolving(), 0);
  }
};

} // namespace actions

} // namespace state
