module;
#include <utility>
#include <cstddef>

export module carcer.actions.world:TownEnemySeekAndMelee;
export import carcer.state;
import :PerformTownMeleeAttack;
import carcer.game.map;
import carcer.game.combat;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

// One agitated enemy: optional seek step, then town melee if adjacent.
class TownEnemySeekAndMelee : public AbstractAction {
  bmin::String enemyId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, database);
    auto* enemy = orch.findCharacterById(enemyId);
    auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (enemy == nullptr || avatar == nullptr) {
      return;
    }

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertAction(new PerformTownMeleeAttack(enemyId), 0);
      return;
    }

    auto dx = 0;
    auto dy = 0;
    if (!game::chooseSeekStepToward(
            state->world.activeMap,
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
    LOG(DEBUG) << "TownEnemyAi: " << enemyId << " stepped (" << dx << ", " << dy << ")"
               << LOG_ENDL;

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertAction(new PerformTownMeleeAttack(enemyId), 0);
    }
  }

public:
  explicit TownEnemySeekAndMelee(bmin::String _enemyId) : enemyId(std::move(_enemyId)) {}
};

} // namespace actions

} // namespace state

} // export
