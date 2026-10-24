#include "game/combat/EnemyBehavior.h"
#include "bmin/StringInterop.h"
#include "game/combat/SpellRules.h"
#include "game/combat/SpellZone.h"
#include "game/combat/StatusRules.h"
#include "game/map/ActiveMapCharacters.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/MapPathfinding.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileDistance.h"
#include "game/map/TileTriggers.h"
#include "model/Combat.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Spells.hpp"
#include "sdl2w/Logger.h"
#include <cstdlib>

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
    if (!isCharacterOnActiveLayer(character, world.activeMap)) {
      continue;
    }
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
    if (!isCharacterOnActiveLayer(townsfolk, world.activeMap)) {
      continue;
    }
    if (townsfolk.agitated || !isTownsperson(townsfolk)) {
      continue;
    }

    for (size_t j = 0; j < world.activeMap.characters.size(); j++) {
      const auto& other = world.activeMap.characters[j];
      if (!isCharacterOnActiveLayer(other, world.activeMap)) {
        continue;
      }
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
    if (!isCharacterOnActiveLayer(character, activeMap)) {
      continue;
    }
    if (character.agitated || character.agitationGroup.empty()) {
      continue;
    }

    for (size_t j = 0; j < activeMap.characters.size(); j++) {
      const auto& other = activeMap.characters[j];
      if (!isCharacterOnActiveLayer(other, activeMap)) {
        continue;
      }
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
  if (!isCharacterOnActiveLayer(enemy, world.activeMap)) {
    return false;
  }

  auto* avatar = findPartyAvatarOnActiveMap(world.activeMap, player);
  if (avatar == nullptr) {
    return false;
  }
  if (!isCharacterOnActiveLayer(*avatar, world.activeMap)) {
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
    if (!isCharacterOnActiveLayer(other, world.activeMap)) {
      continue;
    }
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
    if (!isCharacterOnActiveLayer(other, world.activeMap)) {
      continue;
    }
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

namespace {

bool actorKnowsSpell(const model::CharacterTemplateCombatSpellAi& ai,
                     const bmin::String& spellName) {
  if (spellName.empty()) {
    return false;
  }
  for (size_t i = 0; i < ai.spells.size(); i++) {
    if (ai.spells[i] == spellName) {
      return true;
    }
  }
  return false;
}

bool shouldSkipChosenSpell(int skipChance) {
  if (skipChance <= 0) {
    return false;
  }
  if (skipChance >= 100) {
    return true;
  }
  return (std::rand() % 100) < skipChance;
}

int pickWeightedIndex(const bmin::DynArray<int>& weights) {
  auto total = 0;
  for (size_t i = 0; i < weights.size(); i++) {
    if (weights[i] > 0) {
      total += weights[i];
    }
  }
  if (total <= 0) {
    return -1;
  }
  auto remaining = std::rand() % total;
  for (size_t i = 0; i < weights.size(); i++) {
    const auto weight = weights[i] > 0 ? weights[i] : 0;
    if (weight <= 0) {
      continue;
    }
    if (remaining < weight) {
      return static_cast<int>(i);
    }
    remaining -= weight;
  }
  return static_cast<int>(weights.size()) - 1;
}

model::CombatBehaviorName pickCombatTurnBehavior(const model::CharacterInstance& actor) {
  const auto& modes = actor.combatSpellAi.turnModes;
  if (modes.empty()) {
    return actor.combatBehaviorCombat;
  }
  bmin::DynArray<int> weights;
  for (size_t i = 0; i < modes.size(); i++) {
    weights.pushBack(modes[i].weight);
  }
  const auto index = pickWeightedIndex(weights);
  if (index < 0 || index >= static_cast<int>(modes.size())) {
    return actor.combatBehaviorCombat;
  }
  return modes[static_cast<size_t>(index)].behavior;
}

int zoneHostileCount(model::World& world,
                     const model::Player& player,
                     const model::CharacterInstance& caster,
                     int centerX,
                     int centerY,
                     int zoneW,
                     int zoneH) {
  auto count = 0;
  for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
    const auto& other = world.activeMap.characters[i];
    if (!isCharacterOnActiveLayer(other, world.activeMap)) {
      continue;
    }
    if (other.id == caster.id) {
      continue;
    }
    if (model::isCharacterDefeated(player, other)) {
      continue;
    }
    if (model::isCharacterAlly(player, caster) == model::isCharacterAlly(player, other)) {
      continue;
    }
    if (tileInCenteredZone(other.x, other.y, centerX, centerY, zoneW, zoneH)) {
      count += 1;
    }
  }
  return count;
}

bool characterInSpellRange(const model::CharacterInstance& actor,
                           const model::CharacterInstance& other,
                           int range) {
  const auto dist = chebyshevDistance(actor.x, actor.y, other.x, other.y);
  if (dist > range && range > 0) {
    return false;
  }
  if (range <= 0 && dist > 0) {
    return false;
  }
  return true;
}

bool isHostileZoneAimTarget(model::World& world,
                            const model::Player& player,
                            const model::CharacterInstance& actor,
                            const model::CharacterInstance& other,
                            int range,
                            int zoneW,
                            int zoneH,
                            int required) {
  if (!isCharacterOnActiveLayer(other, world.activeMap)) {
    return false;
  }
  if (other.id == actor.id) {
    return false;
  }
  if (model::isCharacterDefeated(player, other)) {
    return false;
  }
  if (model::isCharacterAlly(player, actor) == model::isCharacterAlly(player, other)) {
    return false;
  }
  if (!characterInSpellRange(actor, other, range)) {
    return false;
  }
  return zoneHostileCount(world, player, actor, other.x, other.y, zoneW, zoneH) >=
         required;
}

int conditionMinZoneHostiles(const model::CombatSpellAiRule& rule) {
  auto minCount = 0;
  for (size_t i = 0; i < rule.conditions.size(); i++) {
    if (rule.conditions[i].type ==
        model::CombatSpellAiConditionType::ZONE_HOSTILE_COUNT_AT_LEAST) {
      if (rule.conditions[i].value > minCount) {
        minCount = rule.conditions[i].value;
      }
    }
  }
  return minCount;
}

bool evaluateNonTargetConditions(const model::CombatSpellAiRule& rule,
                                 model::World& world,
                                 const model::Player& player,
                                 const model::CharacterInstance& actor) {
  (void)world;
  for (size_t i = 0; i < rule.conditions.size(); i++) {
    const auto& condition = rule.conditions[i];
    switch (condition.type) {
    case model::CombatSpellAiConditionType::ALWAYS:
    case model::CombatSpellAiConditionType::ZONE_HOSTILE_COUNT_AT_LEAST:
      break;
    case model::CombatSpellAiConditionType::SELF_HP_BELOW_PERCENT: {
      const auto hp = model::getCharacterHp(player, actor);
      const auto maxHp = model::getCharacterMaxHp(player, actor);
      if (maxHp <= 0 || hp * 100 >= condition.value * maxHp) {
        return false;
      }
      break;
    }
    case model::CombatSpellAiConditionType::SELF_HP_ABOVE_PERCENT: {
      const auto hp = model::getCharacterHp(player, actor);
      const auto maxHp = model::getCharacterMaxHp(player, actor);
      if (maxHp <= 0 || hp * 100 <= condition.value * maxHp) {
        return false;
      }
      break;
    }
    case model::CombatSpellAiConditionType::HAS_STATUS:
      if (!hasAppliedStatus(actor, condition.status)) {
        return false;
      }
      break;
    case model::CombatSpellAiConditionType::LACKS_STATUS:
      if (hasAppliedStatus(actor, condition.status)) {
        return false;
      }
      break;
    case model::CombatSpellAiConditionType::OWN_TURN_AT_MOST:
      if (actor.combatTurnsTaken > condition.value) {
        return false;
      }
      break;
    }
  }
  return true;
}

bool abilityHasRestore(const model::AbilityTemplate& ability) {
  return !ability.restores.empty();
}

bool chooseSpellTarget(model::World& world,
                       const model::Player& player,
                       const model::CharacterInstance& actor,
                       const model::AbilityTemplate& ability,
                       const model::CombatSpellAiRule& rule,
                       EnemySpellAction& out) {
  const auto& select = ability.targetSelect;
  const auto range = select.range > 0 ? select.range : 0;
  const auto zoneW = game::normalizedZoneSize(select.zoneSize.x);
  const auto zoneH = game::normalizedZoneSize(select.zoneSize.y);
  const auto minHostiles = conditionMinZoneHostiles(rule);

  if (select.targetType == model::TargetSelectType::TARGET_SELF) {
    out.targetChId = actor.id;
    out.targetLoc = {actor.x, actor.y};
    return true;
  }

  if (select.targetType == model::TargetSelectType::TARGET_ZONE) {
    const auto required = minHostiles > 0 ? minHostiles : 1;
    const auto method = rule.zoneTargeting;

    if (method == model::CombatSpellAiZoneTargeting::ONLY_PC) {
      auto found = false;
      auto bestDist = 0;
      auto bestX = actor.x;
      auto bestY = actor.y;
      for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
        const auto& other = world.activeMap.characters[i];
        if (!model::isPartyMember(player, other.id)) {
          continue;
        }
        if (!isHostileZoneAimTarget(
                world, player, actor, other, range, zoneW, zoneH, required)) {
          continue;
        }
        const auto dist = chebyshevDistance(actor.x, actor.y, other.x, other.y);
        if (!found || dist < bestDist) {
          found = true;
          bestDist = dist;
          bestX = other.x;
          bestY = other.y;
        }
      }
      if (!found) {
        return false;
      }
      out.targetChId = bmin::String{};
      out.targetLoc = {bestX, bestY};
      return true;
    }

    if (method == model::CombatSpellAiZoneTargeting::RANDOM_IN_RANGE) {
      bmin::DynArray<int> xs;
      bmin::DynArray<int> ys;
      for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
        const auto& other = world.activeMap.characters[i];
        if (!isHostileZoneAimTarget(
                world, player, actor, other, range, zoneW, zoneH, required)) {
          continue;
        }
        xs.pushBack(other.x);
        ys.pushBack(other.y);
      }
      if (xs.empty()) {
        return false;
      }
      const auto pick = std::rand() % static_cast<int>(xs.size());
      out.targetChId = bmin::String{};
      out.targetLoc = {xs[static_cast<size_t>(pick)], ys[static_cast<size_t>(pick)]};
      return true;
    }

    auto found = false;
    auto bestCount = 0;
    auto bestDist = 0;
    auto bestX = actor.x;
    auto bestY = actor.y;
    for (int y = actor.y - range; y <= actor.y + range; y++) {
      for (int x = actor.x - range; x <= actor.x + range; x++) {
        if (chebyshevDistance(actor.x, actor.y, x, y) > range) {
          continue;
        }
        const auto count =
            zoneHostileCount(world, player, actor, x, y, zoneW, zoneH);
        if (count < required) {
          continue;
        }
        const auto dist = chebyshevDistance(actor.x, actor.y, x, y);
        if (!found || count > bestCount || (count == bestCount && dist < bestDist)) {
          found = true;
          bestCount = count;
          bestDist = dist;
          bestX = x;
          bestY = y;
        }
      }
    }
    if (!found) {
      return false;
    }
    out.targetChId = bmin::String{};
    out.targetLoc = {bestX, bestY};
    return true;
  }

  if (select.targetType == model::TargetSelectType::TARGET_UNIT ||
      select.targetType == model::TargetSelectType::TARGET_ALLY) {
    const auto allegiance =
        select.targetType == model::TargetSelectType::TARGET_ALLY
            ? model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME_AND_SELF
            : select.allegianceSelectType;
    const auto preferLowestHp = abilityHasRestore(ability);
    auto found = false;
    auto bestHp = 0;
    auto bestDist = 0;
    const model::CharacterInstance* best = nullptr;
    for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
      const auto& other = world.activeMap.characters[i];
      if (!isCharacterOnActiveLayer(other, world.activeMap)) {
        continue;
      }
      if (model::isCharacterDefeated(player, other)) {
        continue;
      }
      if (!model::matchesSpellAllegiance(player, actor, other, allegiance)) {
        continue;
      }
      const auto dist = chebyshevDistance(actor.x, actor.y, other.x, other.y);
      if (dist > range && range > 0) {
        continue;
      }
      if (range <= 0 && dist > 0) {
        continue;
      }
      const auto hp = model::getCharacterHp(player, other);
      if (!found) {
        found = true;
        best = &other;
        bestHp = hp;
        bestDist = dist;
        continue;
      }
      if (preferLowestHp) {
        if (hp < bestHp || (hp == bestHp && dist < bestDist)) {
          best = &other;
          bestHp = hp;
          bestDist = dist;
        }
      } else if (dist < bestDist) {
        best = &other;
        bestDist = dist;
      }
    }
    if (!found || best == nullptr) {
      return false;
    }
    out.targetChId = best->id;
    out.targetLoc = {best->x, best->y};
    return true;
  }

  return false;
}

bool ruleIsEligible(model::World& world,
                    model::Player& player,
                    model::CharacterInstance& actor,
                    const db::Database& database,
                    const model::CombatSpellAiRule& rule,
                    EnemySpellAction& out) {
  if (!actorKnowsSpell(actor.combatSpellAi, rule.spell)) {
    return false;
  }
  if (actor.currentAp <= 0) {
    return false;
  }
  const auto* spell = database.findSpellTemplate(bmin::toStringView(rule.spell));
  if (spell == nullptr) {
    return false;
  }
  const auto* ability =
      database.findAbilityTemplate(bmin::toStringView(spell->abilityName));
  if (ability == nullptr) {
    return false;
  }
  auto npcStats = model::CharacterStats{};
  auto casterRef = model::SpellCasterRef{};
  if (!model::resolveCombatSpellCaster(player, actor, database, npcStats, casterRef)) {
    return false;
  }
  if (!model::canCastSpell(casterRef, bmin::toStringView(rule.spell), database)) {
    return false;
  }
  if (!evaluateNonTargetConditions(rule, world, player, actor)) {
    return false;
  }
  out.spellName = rule.spell;
  return chooseSpellTarget(world, player, actor, *ability, rule, out);
}

} // namespace

model::CombatBehaviorName resolveCombatTurnBehavior(model::CharacterInstance& actor) {
  if (actor.combatBehaviorChosenTurn == actor.combatTurnsTaken) {
    return actor.combatBehaviorThisTurn;
  }
  actor.combatBehaviorThisTurn = pickCombatTurnBehavior(actor);
  actor.combatBehaviorChosenTurn = actor.combatTurnsTaken;
  return actor.combatBehaviorThisTurn;
}

bool chooseEnemySpellAction(model::World& world,
                            MapInstanceStore& /*mapInstances*/,
                            model::Player& player,
                            model::CharacterInstance& actor,
                            const db::Database& database,
                            EnemySpellAction& out) {
  out = EnemySpellAction{};
  const auto& ai = actor.combatSpellAi;
  if (ai.spells.empty()) {
    return false;
  }

  bmin::DynArray<model::CombatSpellAiRule> implicitRules;
  const auto* rules = &ai.rules;
  if (ai.rules.empty()) {
    for (size_t i = 0; i < ai.spells.size(); i++) {
      model::CombatSpellAiRule rule;
      rule.spell = ai.spells[i];
      rule.zoneTargeting = ai.zoneTargeting;
      implicitRules.pushBack(rule);
    }
    rules = &implicitRules;
  }

  bmin::DynArray<EnemySpellAction> eligible;
  bmin::DynArray<int> weights;
  for (size_t i = 0; i < rules->size(); i++) {
    EnemySpellAction candidate;
    if (!ruleIsEligible(world, player, actor, database, (*rules)[i], candidate)) {
      continue;
    }
    if (ai.selection == model::CombatSpellAiSelection::FIRST) {
      if (shouldSkipChosenSpell(ai.skipChance)) {
        return false;
      }
      out = candidate;
      return true;
    }
    eligible.pushBack(candidate);
    weights.pushBack((*rules)[i].weight);
  }

  if (eligible.empty()) {
    return false;
  }
  if (shouldSkipChosenSpell(ai.skipChance)) {
    return false;
  }
  const auto index = pickWeightedIndex(weights);
  if (index < 0 || index >= static_cast<int>(eligible.size())) {
    return false;
  }
  out = eligible[static_cast<size_t>(index)];
  return true;
}

bool tryQueueTownEnemySpell(model::World& /*world*/,
                            MapInstanceStore& /*mapInstances*/,
                            const model::Player& /*player*/,
                            const model::CharacterInstance& /*enemy*/,
                            const db::Database& /*database*/) {
  return false;
}

} // namespace game
