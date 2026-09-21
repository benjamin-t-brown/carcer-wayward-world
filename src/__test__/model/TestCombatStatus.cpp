#include "db/Database.h"
#include "game/combat/StatusRules.h"
#include "game/map/MapPersistence.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/World.hpp"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/StatusEffects.hpp"
#include "model/templates/Spells.hpp"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/combat/PerformSpellCast.hpp"
#include "actions/combat/SetActiveCombatCharacter.hpp"
#include "actions/combat/TickCombatStatuses.hpp"
#include "state/WorldUpdater.h"
#include "bmin/String.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

void addWalkableTileset(db::Database& database);
void addSingeBurning(db::Database& database);

void pumpActions(state::StateManager& stateManager, int maxMs = 4000) {
  for (int elapsed = 0; elapsed < maxMs; elapsed += 50) {
    stateManager.update(50);
    const auto& actions = stateManager.getActionData();
    if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
        actions.insertActions.empty()) {
      return;
    }
  }
}

int pumpActionsMaxParticles(state::StateManager& stateManager, int maxMs = 4000) {
  auto maxParticles = 0;
  for (int elapsed = 0; elapsed < maxMs; elapsed += 50) {
    stateManager.update(50);
    const auto count =
        static_cast<int>(stateManager.getState().world.activeMap.damageParticles.size());
    if (count > maxParticles) {
      maxParticles = count;
    }
    const auto& actions = stateManager.getActionData();
    if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
        actions.insertActions.empty()) {
      return maxParticles;
    }
  }
  return maxParticles;
}

void pumpCombat(state::StateManager& stateManager, int maxMs = 4000) {
  for (int elapsed = 0; elapsed < maxMs; elapsed += 50) {
    stateManager.update(50);
    state::worldUpdate(nullptr, stateManager, 50);
    const auto& actions = stateManager.getActionData();
    if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
        actions.insertActions.empty()) {
      return;
    }
  }
}

model::CharacterInstance* findCharacter(model::ActiveMap& activeMap, const bmin::String& id) {
  for (auto& ch : activeMap.characters) {
    if (ch.id == id) {
      return &ch;
    }
  }
  return nullptr;
}

bool testLethalBurnSkipsEnemyTurn() {
  db::Database database;
  addSingeBurning(database);
  addWalkableTileset(database);
  state::DatabaseInterface::setDatabase(&database);
  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  auto& state = stateManager.getState();

  auto map = model::MapInstance{};
  map.id = "status_map";
  map.templateName = "status_map";
  map.width = 5;
  map.height = 5;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (auto y = 0; y < 5; y++) {
    for (auto x = 0; x < 5; x++) {
      auto tile = model::TileInstance{};
      tile.x = x;
      tile.y = y;
      tile.tilesetName = "test_terrain";
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  state.mapInstances[map.templateName] = std::move(map);
  model::MapGridTemplate grid;
  grid.name = "status_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 5;
  grid.mapHeight = 5;
  grid.cells = {{"status_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "status_grid";

  model::CharacterPlayer ally;
  ally.instanceId = "ally-1";
  ally.name = "Hero";
  ally.currentHp = 100;
  state.player.party.pushBack(ally);

  auto caster = model::CharacterInstance{};
  caster.id = "ally-1";
  caster.name = "Hero";
  caster.x = 1;
  caster.y = 1;
  caster.currentHp = 100;
  caster.maxHp = 100;
  caster.hpInitialized = true;
  caster.currentAp = model::COMBAT_STARTING_AP;
  state.world.activeMap.characters.pushBack(std::move(caster));

  auto enemy = model::CharacterInstance{};
  enemy.id = "enemy-1";
  enemy.name = "Goblin";
  enemy.type = model::CharacterTemplateType::ENEMY;
  enemy.x = 4;
  enemy.y = 1;
  enemy.currentHp = 2;
  enemy.maxHp = 20;
  enemy.hpInitialized = true;
  enemy.currentAp = model::COMBAT_STARTING_AP;
  enemy.combatBehaviorCombat = model::CombatBehaviorName::SEEK_AND_MELEE;
  model::AbilityStatus applyBurn;
  applyBurn.statusEffect = "BURNING";
  if (!game::applyStatusEffect(enemy, applyBurn, model::CharacterStats{}, database)) {
    LOG(ERROR) << "failed to apply burning for lethal test" << LOG_ENDL;
    return false;
  }
  state.world.activeMap.characters.pushBack(std::move(enemy));

  auto& combat = state.world.combat;
  combat.active = true;
  combat.turnOrderIds.pushBack("enemy-1");
  combat.turnOrderIds.pushBack("ally-1");
  combat.activeTurnIndex = 0;
  state.turnMode = model::TurnMode::TURN_COMBAT;

  stateManager.enqueueAction(
      state::makeAction<state::actions::SetActiveCombatCharacter>("enemy-1", false), 0);
  stateManager.enqueueAction(
      state::makeAction<state::actions::TickCombatStatuses>("enemy-1"), 0);
  pumpActions(stateManager);

  auto ok = true;
  auto* enemyAfterTick = findCharacter(state.world.activeMap, "enemy-1");
  ok = assertTrue(enemyAfterTick != nullptr, "enemy still on map after burn tick") && ok;
  if (enemyAfterTick) {
    ok = assertEqual(enemyAfterTick->x, 4, "burn tick does not move the enemy") && ok;
    ok = assertTrue(model::isCharacterDefeated(state.player, *enemyAfterTick),
                    "enemy defeated by burning") &&
         ok;
  }

  pumpCombat(stateManager);
  ok = assertTrue(combat.activeCharacterId != "enemy-1",
                  "defeated enemy is not the active combatant") &&
       ok;
  auto* enemyAfter = findCharacter(state.world.activeMap, "enemy-1");
  if (enemyAfter != nullptr) {
    ok = assertEqual(enemyAfter->x, 4, "defeated enemy did not move") && ok;
  }
  ok = assertEqual(state.player.party[0].currentHp, 100, "enemy did not attack") && ok;
  return ok;
}

void addWalkableTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  auto meta = model::TileMetadata{};
  meta.id = 0;
  meta.isWalkable = true;
  meta.isSeeThrough = true;
  tileset.tiles.pushBack(meta);
  database.addTilesetTemplate(tileset);
}

void addSingeBurning(db::Database& database) {
  model::StatusEffectTemplate burning;
  burning.name = "BURNING";
  burning.baseDuration = 3;
  model::StatusEffectAction action;
  action.statusActionTargetType = model::StatusActionTargetType::STATUS_ACTION_TARGET_SELF;
  action.abilityName = "SE_BURNING_TEST";
  model::StatusEffectEvent onApplied;
  onApplied.type = model::StatusEventType::STATUS_EVENT_ON_APPLIED;
  onApplied.condition = model::StatusEffectCondition::CONDITION_ALWAYS;
  action.events.pushBack(onApplied);
  model::StatusEffectEvent onTurnStart;
  onTurnStart.type = model::StatusEventType::STATUS_EVENT_ON_TURN_START;
  onTurnStart.condition = model::StatusEffectCondition::CONDITION_ALWAYS;
  action.events.pushBack(onTurnStart);
  burning.actions.pushBack(std::move(action));
  database.addStatusEffectTemplate(burning);

  model::AbilityTemplate burnTick;
  burnTick.name = "SE_BURNING_TEST";
  burnTick.type = model::AbilityType::ABILITY_SPELL;
  burnTick.targetSelect.targetType = model::TargetSelectType::TARGET_SELF;
  burnTick.depiction.dmgAnim = "splash_fire";
  model::AbilityDamage burnDmg;
  burnDmg.damageType = model::DamageType::DAMAGE_TYPE_HEAT;
  burnDmg.dmgDice = {model::Dice::D0};
  burnDmg.dmgBonus = 3;
  burnDmg.dmgStat = model::StatsEnum::STAT_MND;
  burnDmg.dmgStatMult = 0.f;
  burnTick.damages.pushBack(burnDmg);
  database.addAbilityTemplate(burnTick);

  model::AbilityTemplate singe;
  singe.name = "SPELL_SINGE_STATUS";
  singe.type = model::AbilityType::ABILITY_SPELL;
  singe.targetSelect.targetType = model::TargetSelectType::TARGET_ZONE;
  singe.targetSelect.zoneSize = {.x = 1, .y = 1};
  singe.apCost = 0;
  singe.depiction.dmgAnim = "splash_fire";
  model::AbilityStatus applyBurn;
  applyBurn.statusEffect = "BURNING";
  singe.statuses.pushBack(applyBurn);
  database.addAbilityTemplate(singe);

  model::SpellTemplate spell;
  spell.name = "SINGE_STATUS";
  spell.abilityName = "SPELL_SINGE_STATUS";
  database.addSpellTemplate(spell);
}

} // namespace

int main() {
  LOG(INFO) << "Starting TestCombatStatus" << LOG_ENDL;
  auto ok = true;

  {
    model::StatusEffectTemplate templ;
    templ.baseDuration = 3;
    model::AbilityStatus apply;
    apply.statusEffect = "BURNING";
    auto stats = model::CharacterStats{};
    ok = assertEqual(game::calculateStatusDuration(apply, templ, stats),
                     3,
                     "template baseDuration") &&
         ok;
    apply.baseDuration = 5;
    apply.durationBonus = 2;
    ok = assertEqual(game::calculateStatusDuration(apply, templ, stats),
                     7,
                     "ability overrides duration") &&
         ok;
    apply.baseDuration.reset();
    apply.durationBonus.reset();
    templ.durationScale = model::StatusEffectDurationScale{
        .durationStat = model::StatsEnum::STAT_MND,
        .durationStatMult = 0.25f,
    };
    stats.generic.mnd = 8;
    ok = assertEqual(game::calculateStatusDuration(apply, templ, stats),
                     5,
                     "durationScale adds truncated stat * mult") &&
         ok;
  }

  db::Database database;
  addSingeBurning(database);
  addWalkableTileset(database);
  state::DatabaseInterface::setDatabase(&database);
  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  auto& state = stateManager.getState();

  auto map = model::MapInstance{};
  map.id = "status_map";
  map.templateName = "status_map";
  map.width = 5;
  map.height = 5;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (auto y = 0; y < 5; y++) {
    for (auto x = 0; x < 5; x++) {
      auto tile = model::TileInstance{};
      tile.x = x;
      tile.y = y;
      tile.tilesetName = "test_terrain";
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  state.mapInstances[map.templateName] = std::move(map);
  model::MapGridTemplate grid;
  grid.name = "status_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 5;
  grid.mapHeight = 5;
  grid.cells = {{"status_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "status_grid";

  model::CharacterPlayer ally;
  ally.instanceId = "ally-1";
  ally.name = "Hero";
  ally.currentHp = 100;
  state.player.party.pushBack(ally);

  auto caster = model::CharacterInstance{};
  caster.id = "ally-1";
  caster.name = "Hero";
  caster.x = 1;
  caster.y = 1;
  caster.currentHp = 100;
  caster.maxHp = 100;
  caster.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(caster));

  auto enemy = model::CharacterInstance{};
  enemy.id = "enemy-1";
  enemy.name = "Goblin";
  enemy.type = model::CharacterTemplateType::ENEMY;
  enemy.x = 2;
  enemy.y = 1;
  enemy.currentHp = 20;
  enemy.maxHp = 20;
  enemy.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(enemy));

  model::SpellTargetInfo target;
  target.tileX = 2;
  target.tileY = 1;
  stateManager.enqueueAction(
      state::makeAction<state::actions::PerformSpellCast>("ally-1", "SINGE_STATUS", target),
      0);
  const auto maxParticles = pumpActionsMaxParticles(stateManager);

  model::CharacterInstance* enemyPtr = nullptr;
  for (auto& ch : state.world.activeMap.characters) {
    if (ch.id == "enemy-1") {
      enemyPtr = &ch;
      break;
    }
  }
  ok = assertTrue(enemyPtr != nullptr, "enemy after singe") && ok;
  ok = assertEqual(maxParticles, 1, "status-only spell skips its own dmg particle") &&
       ok;
  if (enemyPtr) {
    ok = assertEqual(model::getCharacterHp(state.player, *enemyPtr),
                     17,
                     "on-applied burn 3 only") &&
         ok;
    ok = assertEqual(static_cast<int>(enemyPtr->statusEffects.size()),
                     1,
                     "burning applied") &&
         ok;
    if (!enemyPtr->statusEffects.empty()) {
      ok = assertTrue(enemyPtr->statusEffects[0].statusEffectName == "BURNING",
                      "status name BURNING") &&
           ok;
      ok = assertEqual(enemyPtr->statusEffects[0].remainingTurns,
                       3,
                       "burning lasts 3 turns") &&
           ok;
    }
  }

  state.world.combat.active = true;
  stateManager.enqueueAction(
      state::makeAction<state::actions::SetActiveCombatCharacter>("enemy-1"), 0);
  pumpActions(stateManager);
  enemyPtr = nullptr;
  for (auto& ch : state.world.activeMap.characters) {
    if (ch.id == "enemy-1") {
      enemyPtr = &ch;
      break;
    }
  }
  if (enemyPtr) {
    ok = assertEqual(model::getCharacterHp(state.player, *enemyPtr),
                     17,
                     "reactivate after move does not tick burning") &&
         ok;
    if (!enemyPtr->statusEffects.empty()) {
      ok = assertEqual(enemyPtr->statusEffects[0].remainingTurns,
                       3,
                       "duration unchanged after reactivate") &&
           ok;
    }
  }

  stateManager.enqueueAction(
      state::makeAction<state::actions::TickCombatStatuses>("enemy-1"), 0);
  pumpActions(stateManager);
  enemyPtr = nullptr;
  for (auto& ch : state.world.activeMap.characters) {
    if (ch.id == "enemy-1") {
      enemyPtr = &ch;
      break;
    }
  }
  if (enemyPtr) {
    ok = assertEqual(model::getCharacterHp(state.player, *enemyPtr),
                     14,
                     "turn start burn 3") &&
         ok;
    ok = assertEqual(static_cast<int>(enemyPtr->statusEffects.size()), 1, "still burning") &&
         ok;
    if (!enemyPtr->statusEffects.empty()) {
      ok = assertEqual(enemyPtr->statusEffects[0].remainingTurns,
                       2,
                       "duration decremented at turn start") &&
           ok;
    }
  }

  stateManager.enqueueAction(
      state::makeAction<state::actions::TickCombatStatuses>("enemy-1"), 0);
  pumpActions(stateManager);
  stateManager.enqueueAction(
      state::makeAction<state::actions::TickCombatStatuses>("enemy-1"), 0);
  pumpActions(stateManager);
  enemyPtr = nullptr;
  for (auto& ch : state.world.activeMap.characters) {
    if (ch.id == "enemy-1") {
      enemyPtr = &ch;
      break;
    }
  }
  if (enemyPtr) {
    ok = assertEqual(model::getCharacterHp(state.player, *enemyPtr),
                     8,
                     "two more burn ticks") &&
         ok;
    ok = assertEqual(static_cast<int>(enemyPtr->statusEffects.size()),
                     0,
                     "burning expired") &&
         ok;
  }

  ok = testLethalBurnSkipsEnemyTurn() && ok;

  if (!ok) {
    LOG(ERROR) << "TestCombatStatus failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestCombatStatus completed successfully" << LOG_ENDL;
  return 0;
}
