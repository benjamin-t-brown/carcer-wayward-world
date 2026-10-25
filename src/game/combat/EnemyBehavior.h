#pragma once

#include "db/Database.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileDistance.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/Player.h"
#include "model/instances/World.hpp"

namespace game {

bool canEnemySpotPartyAvatar(model::World& world,
                             MapInstanceStore& mapInstances,
                             const model::Player& player,
                             const model::CharacterInstance& enemy,
                             const db::Database& database);

// Updates spotting / contagion. Returns true when an enemy just became the
// first agitated character on the map (callers should play "roar").
bool updateAgitation(model::World& world,
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
enum class EnemyShootChoice {
  Unavailable,
  Wait,
  Move,
  Shoot,
};

struct EnemyShootDecision {
  EnemyShootChoice choice = EnemyShootChoice::Unavailable;
  int dx = 0;
  int dy = 0;
  bmin::String targetChId;
  int targetX = 0;
  int targetY = 0;
  int range = 0;
};

/**
 * SEEK_AND_SHOOT: shoot the chosen party member when they are in weapon range,
 * otherwise step toward them. Unavailable means the actor cannot fire (no
 * ranged weapon or unpaid aux cost) and the caller should seek-and-melee.
 */
EnemyShootDecision chooseSeekAndShootCombatAction(model::World& world,
                                                  MapInstanceStore& mapInstances,
                                                  const model::Player& player,
                                                  const model::CharacterInstance& actor,
                                                  const db::Database& database);

bool chooseSeekAndMeleeCombatAction(model::World& world,
                                    MapInstanceStore& mapInstances,
                                    const model::Player& player,
                                    const model::CharacterInstance& actor,
                                    const db::Database& database,
                                    int& outDx,
                                    int& outDy);

struct EnemySpellAction {
  bmin::String spellName;
  bmin::String targetChId;
  model::TileXY targetLoc{};
};

/**
 * Choose this actor's combat AI mode for the current turn. Empty turnModes
 * uses combatBehaviorCombat. The pick is cached until combatTurnsTaken changes.
 */
model::CombatBehaviorName resolveCombatTurnBehavior(model::CharacterInstance& actor);

/**
 * Pick a combat spell from actor.combatSpellAi. Returns false if no
 * MP-affordable matching rule (caller should SEEK_AND_MELEE).
 */
bool chooseEnemySpellAction(model::World& world,
                            MapInstanceStore& mapInstances,
                            model::Player& player,
                            model::CharacterInstance& actor,
                            const db::Database& database,
                            EnemySpellAction& out);

/**
 * Town enemy spell cast hook. Returns true if a spell action was queued.
 * v1: always false / queues nothing. Combat uses chooseEnemySpellAction.
 */
bool tryQueueTownEnemySpell(model::World& world,
                            MapInstanceStore& mapInstances,
                            const model::Player& player,
                            const model::CharacterInstance& enemy,
                            const db::Database& database);

} // namespace game
