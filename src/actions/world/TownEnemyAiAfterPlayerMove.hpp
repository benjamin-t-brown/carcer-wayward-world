#pragma once

#include "game/combat/EnemyBehavior.h"
#include "model/templates/CharacterTemplate.h"
#include "state/AbstractAction.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/ClearTownEnemyAiResolving.hpp"
#include "actions/world/TownEnemySeekAndMelee.hpp"
#include <unordered_set>

namespace state {

namespace actions {

// Spotting + one town action per agitated enemy (queued with combat-style delays).
class TownEnemyAiAfterPlayerMove : public AbstractAction {
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

    // Enemies that become agitated from this player move do not act until the
    // next player move / town AI pass.
    std::unordered_set<std::string> alreadyAgitatedEnemyIds;
    for (const auto& character : world.activeMap.characters) {
      if (!character.agitated || !model::characterInstanceIsEnemy(character)) {
        continue;
      }
      alreadyAgitatedEnemyIds.insert(character.id.cStr());
    }

    if (game::updateAgitation(world, state->mapInstances, state->player, *database)) {
      PlaySound("roar").execute(state);
    }

    for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
      const auto& character = world.activeMap.characters[i];
      if (!character.agitated) {
        continue;
      }
      if (!model::characterInstanceIsEnemy(character)) {
        continue;
      }
      if (!alreadyAgitatedEnemyIds.contains(character.id.cStr())) {
        continue;
      }
      if (character.behaviorName != "IMMOBILE_UNTIL_ENEMY_SPOTTED") {
        continue;
      }
      if (character.combatBehaviorTown != model::CombatBehaviorName::SEEK_AND_MELEE) {
        continue;
      }
      insertAction(state::makeAction<TownEnemySeekAndMelee>(character.id), 0);
    }

    insertAction(state::makeAction<ClearTownEnemyAiResolving>(), 0);
  }
};

} // namespace actions

} // namespace state
