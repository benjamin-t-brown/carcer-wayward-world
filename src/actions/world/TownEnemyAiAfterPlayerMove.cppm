module;
#include <cstddef>

export module carcer.actions.world:TownEnemyAiAfterPlayerMove;
export import carcer.state;
import :TownEnemySeekAndMelee;
import :ClearTownEnemyAiResolving;
import carcer.game.map;
import carcer.game.combat;
import carcer.data;
#include "macros.h"

export {

namespace state {

namespace actions {

// Spotting + one town action per agitated enemy (queued with combat-style delays).
class TownEnemyAiAfterPlayerMove : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    auto& world = state->world;
    if (world.combat.active) {
      world.resolvingTownEnemyAi = false;
      return;
    }

    world.resolvingTownEnemyAi = true;
    // Held-move stays active; LayerWorld pauses repeats while this flag is set.

    game::updateEnemySpotting(world, state->player);

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

} // export
