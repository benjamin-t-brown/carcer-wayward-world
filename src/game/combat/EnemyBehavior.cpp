module;
#include <cstddef>
#include <cstdint>
#include <utility>

module carcer.game.combat;
import carcer.game.map;
import carcer.model;
import carcer.state;
import carcer.db;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace game {
namespace {

constexpr const char* kImmobileUntilEnemySpotted = "IMMOBILE_UNTIL_ENEMY_SPOTTED";

bool isImmobileUntilEnemySpotted(const model::CharacterInstance& character) {
  return character.behaviorName == kImmobileUntilEnemySpotted;
}

bool isAiEnemy(const model::CharacterInstance& character) {
  return model::characterInstanceIsEnemy(character) && isImmobileUntilEnemySpotted(character);
}

} // namespace

bool canEnemySpotPartyAvatar(model::World& world,
                             const model::Player& player,
                             const model::CharacterInstance& enemy) {
  if (!model::characterInstanceIsEnemy(enemy)) {
    return false;
  }

  auto* avatar = findPartyAvatarOnActiveMap(world.activeMap, player);
  if (avatar == nullptr) {
    return false;
  }

  const auto dist = chebyshevDistance(enemy.x, enemy.y, avatar->x, avatar->y);
  if (dist > enemy.visionRadius) {
    return false;
  }

  if (world.activeMap.gridId.empty()) {
    return false;
  }

  ActiveMapOrchestrator orch;
  orch.fetchMapGrid(world.activeMap.gridId);
  auto* map = orch.getMapInstanceAt(enemy.x, enemy.y);
  const auto local = orch.activeMapCoordToInstanceCoord(enemy.x, enemy.y);
  if (!map || !local.valid) {
    return false;
  }
  map->tileLayerNumber = world.activeMap.mapLayer;
  return isTileCurrentlyVisible(*map, local.x, local.y);
}

void updateEnemySpotting(model::World& world, const model::Player& player) {
  for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
    auto& character = world.activeMap.characters[i];
    if (character.agitated) {
      continue;
    }
    if (!isAiEnemy(character)) {
      continue;
    }
    if (!canEnemySpotPartyAvatar(world, player, character)) {
      continue;
    }

    character.agitated = true;

    auto displayName = character.name;
    if (displayName.empty()) {
      displayName = character.label;
    }
    if (displayName.empty()) {
      displayName = character.id;
    }

    LOG(INFO) << "EnemySpotting: " << displayName << " (" << character.id
              << ") spotted the player and became agitated at (" << character.x << ", "
              << character.y << ")" << LOG_ENDL;
  }
}

bool chooseSeekStepToward(model::ActiveMap& activeMap,
                          const model::CharacterInstance& actor,
                          int targetX,
                          int targetY,
                          const db::Database& database,
                          int& outDx,
                          int& outDy) {
  outDx = 0;
  outDy = 0;
  const auto startDist = chebyshevDistance(actor.x, actor.y, targetX, targetY);
  if (startDist <= 0) {
    return false;
  }

  const auto reachable = collectReachableTiles(activeMap, actor, 1, database);
  auto bestCheb = startDist;
  auto bestManhattan = 0;
  auto found = false;
  auto bestDx = 0;
  auto bestDy = 0;

  for (size_t i = 0; i < reachable.size(); i++) {
    const auto& tile = reachable[i];
    if (tile.dist != 1) {
      continue;
    }
    const auto cheb = chebyshevDistance(tile.x, tile.y, targetX, targetY);
    if (cheb >= startDist) {
      continue;
    }
    const auto adx = tile.x < targetX ? targetX - tile.x : tile.x - targetX;
    const auto ady = tile.y < targetY ? targetY - tile.y : tile.y - targetY;
    const auto manhattan = adx + ady;
    if (!found || cheb < bestCheb || (cheb == bestCheb && manhattan < bestManhattan)) {
      found = true;
      bestCheb = cheb;
      bestManhattan = manhattan;
      bestDx = tile.x - actor.x;
      bestDy = tile.y - actor.y;
    }
  }

  if (!found) {
    return false;
  }
  outDx = bestDx;
  outDy = bestDy;
  return true;
}

bool chooseSeekAndMeleeCombatAction(model::World& world,
                                    const model::Player& player,
                                    const model::CharacterInstance& actor,
                                    const db::Database& database,
                                    int& outDx,
                                    int& outDy) {
  outDx = 0;
  outDy = 0;

  const auto actorIsEnemy = model::characterInstanceIsEnemy(actor);
  auto bestAdjDist = 0;
  auto foundAdj = false;
  auto adjDx = 0;
  auto adjDy = 0;

  for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
    const auto& other = world.activeMap.characters[i];
    if (other.id == actor.id) {
      continue;
    }
    if (!isChebyshevAdjacent(actor.x, actor.y, other.x, other.y)) {
      continue;
    }
    const auto otherIsEnemy = model::characterInstanceIsEnemy(other);
    if (actorIsEnemy == otherIsEnemy) {
      continue;
    }
    const auto dist = chebyshevDistance(actor.x, actor.y, other.x, other.y);
    if (!foundAdj || dist < bestAdjDist) {
      foundAdj = true;
      bestAdjDist = dist;
      adjDx = other.x - actor.x;
      adjDy = other.y - actor.y;
    }
  }

  if (foundAdj) {
    outDx = adjDx;
    outDy = adjDy;
    return true;
  }

  auto bestTargetDist = 0;
  auto foundTarget = false;
  auto targetX = 0;
  auto targetY = 0;
  for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
    const auto& other = world.activeMap.characters[i];
    if (!model::isPartyMember(player, other.id)) {
      continue;
    }
    auto living = false;
    for (size_t pi = 0; pi < player.party.size(); pi++) {
      if (player.party[pi].instanceId == other.id && player.party[pi].currentHp > 0) {
        living = true;
        break;
      }
    }
    if (!living) {
      continue;
    }
    const auto dist = chebyshevDistance(actor.x, actor.y, other.x, other.y);
    if (!foundTarget || dist < bestTargetDist) {
      foundTarget = true;
      bestTargetDist = dist;
      targetX = other.x;
      targetY = other.y;
    }
  }

  if (!foundTarget) {
    return false;
  }
  return chooseSeekStepToward(
      world.activeMap, actor, targetX, targetY, database, outDx, outDy);
}

} // namespace game
