#include "game/combat/EnemyBehavior.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapPathfinding.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileDistance.h"
#include "game/map/TileTriggers.h"
#include "model/Combat.h"
#include "model/templates/CharacterTemplate.h"
#include "sdl2w/Logger.h"

namespace game {
namespace {

bool isCharacterTilePlayerVisible(model::World& world,
                                  MapInstanceStore& mapInstances,
                                  const model::CharacterInstance& character,
                                  const db::Database& database) {
  if (world.activeMap.gridId.empty()) {
    return false;
  }

  ActiveMapOrchestrator orch(world.activeMap, mapInstances, &database);
  auto* map = orch.getMapInstanceAt(character.x, character.y);
  const auto local = orch.activeMapCoordToInstanceCoord(character.x, character.y);
  if (!map || !local.valid) {
    return false;
  }
  map->tileLayerNumber = world.activeMap.mapLayer;
  return isTileCurrentlyVisible(*map, local.x, local.y);
}

bool isTownsperson(const model::CharacterInstance& character) {
  return character.type == model::CharacterTemplateType::TOWNSPERSON ||
         character.type == model::CharacterTemplateType::TOWNSPERSON_STATIC;
}

bmin::String characterDisplayName(const model::CharacterInstance& character) {
  if (!character.name.empty()) {
    return character.name;
  }
  if (!character.label.empty()) {
    return character.label;
  }
  return character.id;
}

void logAgitated(const model::CharacterInstance& character, const char* reason) {
  LOG(INFO) << "Agitation: " << characterDisplayName(character) << " (" << character.id
            << ") " << reason << " at (" << character.x << ", " << character.y << ")"
            << LOG_ENDL;
}

bool canTownspersonSpotAgitatedEnemy(model::World& world,
                                     MapInstanceStore& mapInstances,
                                     const model::CharacterInstance& townsfolk,
                                     const model::CharacterInstance& enemy,
                                     const db::Database& database) {
  if (!isTownsperson(townsfolk)) {
    return false;
  }
  if (!model::characterInstanceIsEnemy(enemy) || !enemy.agitated) {
    return false;
  }

  const auto dist = chebyshevDistance(townsfolk.x, townsfolk.y, enemy.x, enemy.y);
  if (dist > townsfolk.visionRadius) {
    return false;
  }

  // Townspersons may only spot enemies the player can also see.
  return isCharacterTilePlayerVisible(world, mapInstances, enemy, database);
}

struct EnemySpottingPassResult {
  bool changed = false;
  bool firstEnemyAgitation = false;
};

bool anyCharacterAgitated(const model::ActiveMap& activeMap) {
  for (size_t i = 0; i < activeMap.characters.size(); i++) {
    if (activeMap.characters[i].agitated) {
      return true;
    }
  }
  return false;
}

EnemySpottingPassResult applyEnemySpottingPass(model::World& world,
                                               MapInstanceStore& mapInstances,
                                               const model::Player& player,
                                               const db::Database& database) {
  EnemySpottingPassResult result;
  const auto hadAgitated = anyCharacterAgitated(world.activeMap);
  for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
    auto& character = world.activeMap.characters[i];
    if (character.agitated) {
      continue;
    }
    if (!model::characterInstanceIsEnemy(character)) {
      continue;
    }
    if (!canEnemySpotPartyAvatar(world, mapInstances, player, character, database)) {
      continue;
    }

    character.agitated = true;
    logAgitated(character, "spotted the player");
    result.changed = true;
    if (!hadAgitated) {
      result.firstEnemyAgitation = true;
    }
  }
  return result;
}

bool applyTownspersonSpottingPass(model::World& world,
                                  MapInstanceStore& mapInstances,
                                  const db::Database& database) {
  auto changed = false;
  for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
    auto& townsfolk = world.activeMap.characters[i];
    if (townsfolk.agitated || !isTownsperson(townsfolk)) {
      continue;
    }

    for (size_t j = 0; j < world.activeMap.characters.size(); j++) {
      const auto& other = world.activeMap.characters[j];
      if (townsfolk.id == other.id) {
        continue;
      }
      if (!canTownspersonSpotAgitatedEnemy(
              world, mapInstances, townsfolk, other, database)) {
        continue;
      }

      townsfolk.agitated = true;
      logAgitated(townsfolk, "spotted an agitated enemy");
      changed = true;
      break;
    }
  }
  return changed;
}

bool applyGroupContagionPass(model::ActiveMap& activeMap) {
  auto changed = false;
  for (size_t i = 0; i < activeMap.characters.size(); i++) {
    auto& character = activeMap.characters[i];
    if (character.agitated || character.agitationGroup.empty()) {
      continue;
    }

    for (size_t j = 0; j < activeMap.characters.size(); j++) {
      const auto& other = activeMap.characters[j];
      if (other.id == character.id || !other.agitated) {
        continue;
      }
      if (other.agitationGroup != character.agitationGroup) {
        continue;
      }

      character.agitated = true;
      logAgitated(character, "agitated with their group");
      changed = true;
      break;
    }
  }
  return changed;
}

bool anyLivingAgitatedEnemy(const model::World& world, const model::Player& player) {
  for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
    const auto& character = world.activeMap.characters[i];
    if (!model::characterInstanceIsEnemy(character) || !character.agitated) {
      continue;
    }
    if (model::isCharacterDefeated(player, character)) {
      continue;
    }
    return true;
  }
  return false;
}

bool applyTownspersonCalmPass(model::World& world, const model::Player& player) {
  if (anyLivingAgitatedEnemy(world, player)) {
    return false;
  }

  auto changed = false;
  for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
    auto& character = world.activeMap.characters[i];
    if (!character.agitated || !isTownsperson(character)) {
      continue;
    }
    character.agitated = false;
    logAgitated(character, "calmed; no agitated enemies remain");
    changed = true;
  }
  return changed;
}

} // namespace

bool canEnemySpotPartyAvatar(model::World& world,
                             MapInstanceStore& mapInstances,
                             const model::Player& player,
                             const model::CharacterInstance& enemy,
                             const db::Database& database) {
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

  return isCharacterTilePlayerVisible(world, mapInstances, enemy, database);
}

bool updateAgitation(model::World& world,
                     MapInstanceStore& mapInstances,
                     const model::Player& player,
                     const db::Database& database) {
  auto shouldPlayRoar = false;
  constexpr auto kMaxPasses = 32;
  for (auto pass = 0; pass < kMaxPasses; pass++) {
    const auto enemyResult =
        applyEnemySpottingPass(world, mapInstances, player, database);
    if (enemyResult.firstEnemyAgitation) {
      shouldPlayRoar = true;
    }
    const auto townChanged =
        applyTownspersonSpottingPass(world, mapInstances, database);
    const auto groupChanged = applyGroupContagionPass(world.activeMap);
    const auto calmChanged = applyTownspersonCalmPass(world, player);
    if (!enemyResult.changed && !townChanged && !groupChanged && !calmChanged) {
      break;
    }
  }
  return shouldPlayRoar;
}

bool chooseSeekStepToward(model::ActiveMap& activeMap,
                          MapInstanceStore& mapInstances,
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

  const auto reachable =
      collectReachableTiles(activeMap, mapInstances, actor, 1, database);
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
                                    MapInstanceStore& mapInstances,
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
      world.activeMap,
      mapInstances,
      actor,
      targetX,
      targetY,
      database,
      outDx,
      outDy);
}

} // namespace game
