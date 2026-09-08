#pragma once

#include "game/combat/EnemyBehavior.h"
#include "game/map/TileDistance.h"
#include "model/instances/CharacterInstance.hpp"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "actions/world/PerformTownMeleeAttack.hpp"

namespace state::actions {

// One agitated enemy: optional seek step, then town melee if adjacent.
class TownEnemySeekAndMelee : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::TownEnemySeekAndMelee;
  }

  bmin::String enemyId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (!database) {
      return;
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, database);
    auto* enemy = orch.findCharacterById(enemyId);
    auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (!enemy || !avatar) {
      return;
    }

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertAction(state::makeAction<PerformTownMeleeAttack>(enemyId), 0);
      return;
    }

    auto dx = 0;
    auto dy = 0;
    if (!game::chooseSeekStepToward(state->world.activeMap,
                                    state->mapInstances,
                                    *enemy,
                                    avatar->x,
                                    avatar->y,
                                    *database,
                                    dx,
                                    dy)) {
      return;
    }

    model::updateCharacterFacingFromMove(*enemy, dx, dy);
    enemy->x += dx;
    enemy->y += dy;
    LOG(DEBUG) << "TownEnemyAi: " << enemyId << " stepped (" << dx << ", "
               << dy << ")" << LOG_ENDL;

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertAction(state::makeAction<PerformTownMeleeAttack>(enemyId), 0);
    }
  }

public:
  explicit TownEnemySeekAndMelee(bmin::String enemyId)
      : enemyId(std::move(enemyId)) {}
};

} // namespace state::actions
