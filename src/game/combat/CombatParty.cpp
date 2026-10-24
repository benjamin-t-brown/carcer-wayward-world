#include "game/combat/CombatParty.h"

#include "db/Database.h"
#include "game/map/CharacterConstruction.h"
#include "game/map/MapPathfinding.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/instances/World.hpp"
#include "bmin/DynArray.h"
#include <algorithm>
#include <utility>

namespace game {
namespace {

struct TileOffset {
  int dx = 0;
  int dy = 0;
};

void appendCenteredLaterals(int count, bmin::DynArray<int>& out) {
  if (count <= 0) {
    return;
  }
  if (count % 2 == 1) {
    const int half = count / 2;
    for (int i = -half; i <= half; i++) {
      out.pushBack(i);
    }
    return;
  }
  const int half = count / 2;
  for (int i = -half; i <= half; i++) {
    if (i != 0) {
      out.pushBack(i);
    }
  }
}

bmin::DynArray<TileOffset> computeFormationOffsets(int memberCount,
                                                   int forwardDx,
                                                   int forwardDy) {
  bmin::DynArray<TileOffset> offsets;
  if (memberCount <= 0) {
    return offsets;
  }
  const int behindDx = -forwardDx;
  const int behindDy = -forwardDy;
  const int rightDx = -forwardDy;
  const int rightDy = forwardDx;
  const bool diagonal = forwardDx != 0 && forwardDy != 0;

  int remaining = memberCount;
  int depth = 1;
  while (remaining > 0) {
    const int capacity = depth + 1;
    const int take = std::min(remaining, capacity);
    bmin::DynArray<int> laterals;
    appendCenteredLaterals(take, laterals);

    if (diagonal && depth == 1 && take == 2) {
      // Two rear cardinals of the diagonal — each adjacent to the leader.
      TileOffset a{-forwardDx, 0};
      TileOffset b{0, -forwardDy};
      const int aDot = a.dx * rightDx + a.dy * rightDy;
      const int bDot = b.dx * rightDx + b.dy * rightDy;
      // Lower dot with "right" is the left-hand slot (lateral -1).
      if (aDot <= bDot) {
        offsets.pushBack(a);
        offsets.pushBack(b);
      } else {
        offsets.pushBack(b);
        offsets.pushBack(a);
      }
    } else if (diagonal && depth >= 2) {
      // Same as cardinal depth*behind, then pull one diagonal toward the leader.
      const int pulledDepth = depth - 1;
      for (size_t i = 0; i < laterals.size(); i++) {
        const int lateral = laterals[i];
        offsets.pushBack({lateral * rightDx + pulledDepth * behindDx,
                          lateral * rightDy + pulledDepth * behindDy});
      }
    } else {
      for (size_t i = 0; i < laterals.size(); i++) {
        const int lateral = laterals[i];
        offsets.pushBack({lateral * rightDx + depth * behindDx,
                          lateral * rightDy + depth * behindDy});
      }
    }
    remaining -= take;
    depth += 1;
  }
  return offsets;
}

bmin::DynArray<TileOffset> computeLineOffsets(int memberCount,
                                              int forwardDx,
                                              int forwardDy) {
  bmin::DynArray<TileOffset> offsets;
  const int behindDx = -forwardDx;
  const int behindDy = -forwardDy;
  for (int depth = 1; depth <= memberCount; depth++) {
    offsets.pushBack({depth * behindDx, depth * behindDy});
  }
  return offsets;
}

model::CharacterInstance makePartyMapInstance(const model::CharacterPlayer& member,
                                              int x,
                                              int y,
                                              int mapLayer,
                                              const db::Database& database) {
  auto instance = model::CharacterInstance{};
  instance.id = member.instanceId;
  instance.name = member.name.empty() ? member.params.name : member.name;
  instance.templateName =
      member.templateName.empty() ? member.params.name : member.templateName;
  instance.x = x;
  instance.y = y;
  instance.mapLayer = mapLayer;
  instance.spawnX = x;
  instance.spawnY = y;
  instance.currentAp = model::COMBAT_STARTING_AP;
  instance.currentHp = member.currentHp;
  applyCharacterTemplateFromDatabase(instance, database);
  return instance;
}

bool isCharacterAlreadyOnMap(const model::ActiveMap& activeMap,
                             const bmin::String& instanceId) {
  for (const auto& character : activeMap.characters) {
    if (character.id == instanceId) {
      return true;
    }
  }
  return false;
}

bool canPlaceAtReachableTile(model::ActiveMap& activeMap,
                             MapInstanceStore& mapInstances,
                             int startX,
                             int startY,
                             int targetX,
                             int targetY,
                             const bmin::String& pathingCharacterId,
                             int maxSteps,
                             const db::Database& database) {
  const auto reachable = collectReachableTiles(activeMap,
                                               mapInstances,
                                               startX,
                                               startY,
                                               maxSteps,
                                               pathingCharacterId,
                                               database);
  return isTileInReachableSet(reachable, targetX, targetY);
}

void removeCharactersByIds(model::ActiveMap& activeMap,
                           const bmin::DynArray<bmin::String>& ids) {
  for (size_t i = 0; i < ids.size(); i++) {
    const auto& id = ids[i];
    for (size_t ci = 0; ci < activeMap.characters.size();) {
      if (activeMap.characters[ci].id == id) {
        activeMap.characters.erase(ci);
      } else {
        ci++;
      }
    }
  }
}

bool tryPlaceWithOffsets(model::World& world,
                         MapInstanceStore& mapInstances,
                         const db::Database& database,
                         int leaderX,
                         int leaderY,
                         const bmin::String& leaderId,
                         const bmin::DynArray<const model::CharacterPlayer*>& missing,
                         const bmin::DynArray<TileOffset>& offsets) {
  if (offsets.size() != missing.size()) {
    return false;
  }

  auto& activeMap = world.activeMap;
  bmin::DynArray<bmin::String> placedIds;
  const int maxSteps =
      std::max(8, static_cast<int>(missing.size()) * 4 + 4);

  for (size_t i = 0; i < missing.size(); i++) {
    const int targetX = leaderX + offsets[i].dx;
    const int targetY = leaderY + offsets[i].dy;
    if (!canPlaceAtReachableTile(activeMap,
                                 mapInstances,
                                 leaderX,
                                 leaderY,
                                 targetX,
                                 targetY,
                                 leaderId,
                                 maxSteps,
                                 database)) {
      removeCharactersByIds(activeMap, placedIds);
      return false;
    }

    auto instance = makePartyMapInstance(
        *missing[i], targetX, targetY, activeMap.mapLayer, database);
    placedIds.pushBack(instance.id);
    activeMap.characters.pushBack(std::move(instance));
  }
  return true;
}

void placeStackedOnLeader(model::World& world,
                          const db::Database& database,
                          int leaderX,
                          int leaderY,
                          const bmin::DynArray<const model::CharacterPlayer*>& missing) {
  auto& activeMap = world.activeMap;
  for (size_t i = 0; i < missing.size(); i++) {
    activeMap.characters.pushBack(makePartyMapInstance(
        *missing[i], leaderX, leaderY, activeMap.mapLayer, database));
  }
}

void copyLeaderFacingToParty(model::ActiveMap& activeMap, const model::Player& player) {
  if (player.party.empty()) {
    return;
  }
  const auto& leaderId = player.party[0].instanceId;
  model::CharacterInstance* leader = nullptr;
  for (auto& character : activeMap.characters) {
    if (character.id == leaderId) {
      leader = &character;
      break;
    }
  }
  if (leader == nullptr) {
    return;
  }

  int forwardDx = 0;
  int forwardDy = 0;
  model::unitStepFromDelta(leader->lastMoveDx, leader->lastMoveDy, forwardDx, forwardDy);
  if (forwardDx == 0 && forwardDy == 0) {
    forwardDx = 1;
    forwardDy = 0;
  }

  for (auto& character : activeMap.characters) {
    if (!model::isPartyMember(player, character.id)) {
      continue;
    }
    model::updateCharacterFacingFromMove(character, forwardDx, forwardDy);
  }
}

} // namespace

void addPartyMembersToCombatMap(model::World& world,
                                model::Player& player,
                                MapInstanceStore& mapInstances,
                                const db::Database& database) {
  auto& activeMap = world.activeMap;
  model::CharacterInstance* leader = nullptr;
  if (!player.party.empty()) {
    const auto& leaderId = player.party[0].instanceId;
    for (auto& character : activeMap.characters) {
      if (character.id == leaderId) {
        leader = &character;
        break;
      }
    }
  }

  bmin::DynArray<const model::CharacterPlayer*> missing;
  for (size_t i = 0; i < player.party.size(); i++) {
    const auto& member = player.party[i];
    if (isCharacterAlreadyOnMap(activeMap, member.instanceId)) {
      continue;
    }
    missing.pushBack(&member);
  }

  if (!missing.empty() && leader != nullptr) {
    // Copy before pushBack — activeMap.characters may reallocate.
    const int leaderX = leader->x;
    const int leaderY = leader->y;
    const bmin::String leaderId = leader->id;
    int forwardDx = 0;
    int forwardDy = 0;
    model::unitStepFromDelta(leader->lastMoveDx, leader->lastMoveDy, forwardDx, forwardDy);
    if (forwardDx == 0 && forwardDy == 0) {
      forwardDx = 1;
      forwardDy = 0;
    }

    const auto formation =
        computeFormationOffsets(static_cast<int>(missing.size()), forwardDx, forwardDy);
    const bool formed = tryPlaceWithOffsets(world,
                                            mapInstances,
                                            database,
                                            leaderX,
                                            leaderY,
                                            leaderId,
                                            missing,
                                            formation);
    if (!formed) {
      const auto line =
          computeLineOffsets(static_cast<int>(missing.size()), forwardDx, forwardDy);
      const bool lined = tryPlaceWithOffsets(world,
                                             mapInstances,
                                             database,
                                             leaderX,
                                             leaderY,
                                             leaderId,
                                             missing,
                                             line);
      if (!lined) {
        placeStackedOnLeader(world, database, leaderX, leaderY, missing);
      }
    }
  } else if (!missing.empty()) {
    const int spawnX = 0;
    const int spawnY = 0;
    for (size_t i = 0; i < missing.size(); i++) {
      activeMap.characters.pushBack(makePartyMapInstance(
          *missing[i], spawnX, spawnY, activeMap.mapLayer, database));
    }
  }

  for (auto& character : activeMap.characters) {
    if (character.currentHp <= 0 && model::isCharacterEnemy(character)) {
      character.currentHp = character.maxHp;
    }
  }

  copyLeaderFacingToParty(activeMap, player);
}

} // namespace game
