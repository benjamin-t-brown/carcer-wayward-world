#pragma once

#include "game/map/EnemyBehavior.h"
#include "model/instances/CharacterInstance.h"
#include "model/templates/CharacterTemplate.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/world/PerformTownMeleeAttack.hpp"

namespace state {

namespace actions {

class ClearTownEnemyAiResolving : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    state->world.resolvingTownEnemyAi = false;
  }
};

// One agitated enemy: optional seek step, then town melee if adjacent.
class TownEnemySeekAndMelee : public CombatAction {
  bmin::String enemyId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    game::ActiveMapOrchestrator orch;
    auto* enemy = orch.findCharacterById(enemyId);
    auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (enemy == nullptr || avatar == nullptr) {
      return;
    }

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertCombatAction(new PerformTownMeleeAttack(enemyId), 0);
      return;
    }

    auto dx = 0;
    auto dy = 0;
    if (!game::chooseSeekStepToward(
            state->world.activeMap, *enemy, avatar->x, avatar->y, *database, dx, dy)) {
      return;
    }

    model::updateCharacterFacingFromMove(*enemy, dx, dy);
    enemy->x += dx;
    enemy->y += dy;
    LOG(DEBUG) << "TownEnemyAi: " << enemyId << " stepped (" << dx << ", " << dy << ")"
               << LOG_ENDL;

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertCombatAction(new PerformTownMeleeAttack(enemyId), 0);
    }
  }

public:
  explicit TownEnemySeekAndMelee(bmin::String _enemyId) : enemyId(std::move(_enemyId)) {}
};

// Spotting + one town action per agitated enemy (queued with combat-style delays).
class TownEnemyAiAfterPlayerMove : public CombatAction {
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
      insertCombatAction(new TownEnemySeekAndMelee(character.id), 0);
    }

    insertCombatAction(new ClearTownEnemyAiResolving(), 0);
  }
};

} // namespace actions

} // namespace state
