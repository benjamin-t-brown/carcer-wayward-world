#pragma once

#include "db/Database.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/Player.h"
#include "model/instances/World.h"

namespace state {
struct State;
}

namespace game {

int chebyshevDistance(int x0, int y0, int x1, int y1);

bool isChebyshevAdjacent(int x0, int y0, int x1, int y1);

bool canEnemySpotPartyAvatar(model::World& world,
                             const model::Player& player,
                             const model::CharacterInstance& enemy);

void updateEnemySpotting(model::World& world, const model::Player& player);

/**
 * Choose one step (dx, dy) for SEEK_AND_MELEE toward (targetX, targetY).
 * Prefer a reachable neighbor that reduces Chebyshev distance. Returns false if none.
 * database is used only for walkability / pathfinding.
 */
bool chooseSeekStepToward(model::ActiveMap& activeMap,
                          const model::CharacterInstance& actor,
                          int targetX,
                          int targetY,
                          const db::Database& database,
                          int& outDx,
                          int& outDy);

/**
 * Combat SEEK_AND_MELEE: if adjacent to a hostile, move into their tile;
 * else step toward the nearest living party member on the map.
 * database is used only for walkability / pathfinding.
 */
bool chooseSeekAndMeleeCombatAction(model::World& world,
                                    const model::Player& player,
                                    const model::CharacterInstance& actor,
                                    const db::Database& database,
                                    int& outDx,
                                    int& outDy);

/**
 * Enqueues timed town enemy AI (seek / melee with combat swing timing).
 * Requires StateManager; sets world.resolvingTownEnemyAi until the sequence ends.
 */
void runTownEnemyAiAfterPlayerMove(state::State& state, const db::Database& database);

} // namespace game
