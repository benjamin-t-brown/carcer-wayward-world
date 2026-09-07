#pragma once

#include "db/Database.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileDistance.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/Player.h"
#include "model/instances/World.h"

namespace game {

bool canEnemySpotPartyAvatar(model::World& world,
                             MapInstanceStore& mapInstances,
                             const model::Player& player,
                             const model::CharacterInstance& enemy,
                             const db::Database& database);

void updateEnemySpotting(model::World& world,
                         MapInstanceStore& mapInstances,
                         const model::Player& player,
                         const db::Database& database);

/**
 * Choose one step (dx, dy) for SEEK_AND_MELEE toward (targetX, targetY).
 * Prefer a reachable neighbor that reduces Chebyshev distance. Returns false if none.
 * database is used only for walkability / pathfinding.
 */
bool chooseSeekStepToward(model::ActiveMap& activeMap,
                          MapInstanceStore& mapInstances,
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
                                    MapInstanceStore& mapInstances,
                                    const model::Player& player,
                                    const model::CharacterInstance& actor,
                                    const db::Database& database,
                                    int& outDx,
                                    int& outDy);

} // namespace game
