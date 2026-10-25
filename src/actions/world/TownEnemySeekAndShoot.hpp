#pragma once

#include "game/combat/EnemyBehavior.h"
#include "game/map/ActiveMapCharacters.h"
#include "game/map/TileDistance.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "actions/combat/PerformShoot.hpp"
#include "actions/world/TownEnemySeekAndMelee.hpp"

namespace state::actions {

/** One agitated enemy: shoot if in range, otherwise step toward the target. */
class TownEnemySeekAndShoot : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::TownEnemySeekAndShoot; }

  bmin::String enemyId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }
    auto* enemy = game::findCharacterById(state->world.activeMap, enemyId);
    if (enemy == nullptr ||
        !game::isCharacterOnActiveLayer(*enemy, state->world.activeMap)) {
      return;
    }

    const auto decision = game::chooseSeekAndShootCombatAction(
        state->world, state->mapInstances, state->player, *enemy, *database);
    if (decision.choice == game::EnemyShootChoice::Unavailable) {
      insertAction(state::makeAction<TownEnemySeekAndMelee>(enemyId), 0);
      return;
    }
    if (decision.choice == game::EnemyShootChoice::Shoot) {
      model::SpellTargetInfo target;
      target.targetCharacterId = decision.targetChId;
      target.tileX = decision.targetX;
      target.tileY = decision.targetY;
      insertAction(state::makeAction<PerformShoot>(enemyId, target), 0);
      return;
    }
    if (decision.choice != game::EnemyShootChoice::Move) {
      return;
    }

    model::updateCharacterFacingFromMove(*enemy, decision.dx, decision.dy);
    enemy->x += decision.dx;
    enemy->y += decision.dy;
    LOG(DEBUG) << "TownEnemyAi: " << enemyId << " stepped toward a shot ("
               << decision.dx << ", " << decision.dy << ")" << LOG_ENDL;

    const auto dist = game::chebyshevDistance(
        enemy->x, enemy->y, decision.targetX, decision.targetY);
    if (decision.range > 0 && dist > 0 && dist <= decision.range) {
      model::SpellTargetInfo target;
      target.targetCharacterId = decision.targetChId;
      target.tileX = decision.targetX;
      target.tileY = decision.targetY;
      insertAction(state::makeAction<PerformShoot>(enemyId, target), 0);
    }
  }

public:
  explicit TownEnemySeekAndShoot(bmin::String enemyId) : enemyId(std::move(enemyId)) {}
};

} // namespace state::actions
