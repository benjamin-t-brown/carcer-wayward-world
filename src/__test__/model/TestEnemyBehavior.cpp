#include "db/Database.h"
#include "game/combat/EnemyBehavior.h"
#include "game/map/TileDistance.h"
#include "game/map/CharacterConstruction.h"
#include "game/map/MapVision.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/MapInstance.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/world/WorldUpdater.h"
#include "actions/combat/DoCPUCombatTurn.hpp"
#include "actions/combat/StartCombat.hpp"
#include "actions/world/TownEnemyAiAfterPlayerMove.hpp"
#include "actions/world/WorldMovePlayer.hpp"
#include "bmin/String.h"
#include <cstdlib>

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

bool assertFalse(bool cond, const char* label) {
  if (cond) {
    LOG(ERROR) << label << " expected false" << LOG_ENDL;
    return false;
  }
  return true;
}

void tickState(state::StateManager& stateManager, int dt) {
  stateManager.update(dt);
  state::worldUpdate(nullptr, stateManager, dt);
}

void pumpTownEnemyAi(state::StateManager& stateManager, int maxMs = 2000) {
  for (int elapsed = 0; elapsed < maxMs; elapsed += 50) {
    tickState(stateManager, 50);
    const auto& actions = stateManager.getActionData();
    if (!stateManager.getState().world.resolvingTownEnemyAi &&
        actions.sequentialActions.empty() && actions.sequentialActionsNext.empty()) {
      return;
    }
  }
}

void runAndPumpTownEnemyAi(state::StateManager& stateManager, db::Database& database) {
  (void)database;
  stateManager.getState().world.resolvingTownEnemyAi = true;
  stateManager.enqueueAction(stateManager.getActionData(),
                             new state::actions::TownEnemyAiAfterPlayerMove(),
                             0);
  pumpTownEnemyAi(stateManager);
}

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  auto meta = model::TileMetadata{};
  meta.id = 0;
  meta.isWalkable = true;
  meta.isSeeThrough = true;
  tileset.tiles.pushBack(meta);
  database.addTilesetTemplate(tileset);
}

model::MapInstance makeEmptyMap(int width, int height) {
  auto map = model::MapInstance{};
  map.id = "enemy_ai_map";
  map.templateName = "enemy_ai_map";
  map.width = width;
  map.height = height;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  map.tileLayerNumber = 0;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (auto y = 0; y < height; y++) {
    for (auto x = 0; x < width; x++) {
      auto tile = model::TileInstance{};
      tile.x = x;
      tile.y = y;
      tile.tilesetName = "test_terrain";
      tile.tileId = 0;
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  return map;
}

void setupGrid(db::Database& database, state::State& state, int width, int height) {
  addTestTileset(database);
  auto map = makeEmptyMap(width, height);
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "enemy_ai_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = width;
  grid.mapHeight = height;
  grid.cells = {{"enemy_ai_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "enemy_ai_grid";
}

void addHeroTemplate(db::Database& database) {
  auto hero = model::CharacterTemplate{};
  hero.type = model::CharacterTemplateType::TOWNSPERSON;
  hero.name = "hero";
  hero.combat.hp = 100;
  database.addCharacterTemplate(hero);
}

void addGoblinTemplate(db::Database& database, int visionRadius) {
  auto goblin = model::CharacterTemplate{};
  goblin.type = model::CharacterTemplateType::ENEMY;
  goblin.name = "goblinTest";
  goblin.combat.hp = 12;
  goblin.vision.radius = visionRadius;
  goblin.behavior.behaviorName = "IMMOBILE_UNTIL_ENEMY_SPOTTED";
  goblin.combatBehavior.town = model::CombatBehaviorName::SEEK_AND_MELEE;
  goblin.combatBehavior.combat = model::CombatBehaviorName::SEEK_AND_MELEE;
  database.addCharacterTemplate(goblin);
}

model::CharacterInstance*
findOnActiveMap(model::ActiveMap& activeMap, const bmin::String& id) {
  for (auto& ch : activeMap.characters) {
    if (ch.id == id) {
      return &ch;
    }
  }
  return nullptr;
}

state::State makeTownState(db::Database& database,
                           int avatarX,
                           int avatarY,
                           int enemyX,
                           int enemyY,
                           int visionRadius) {
  addHeroTemplate(database);
  addGoblinTemplate(database, visionRadius);
  state::DatabaseInterface::setDatabase(&database);

  state::State state;
  setupGrid(database, state, 12, 12);

  auto leader = model::CharacterPlayer{};
  leader.instanceId = "ally-1";
  leader.name = "Hero";
  leader.templateName = "hero";
  leader.currentHp = 100;
  state.player.party.pushBack(std::move(leader));

  auto member2 = model::CharacterPlayer{};
  member2.instanceId = "ally-2";
  member2.name = "Sidekick";
  member2.templateName = "hero";
  member2.currentHp = 80;
  state.player.party.pushBack(std::move(member2));

  auto avatar = model::CharacterInstance{};
  avatar.id = "ally-1";
  avatar.name = "Hero";
  avatar.templateName = "hero";
  avatar.x = avatarX;
  avatar.y = avatarY;
  state.world.activeMap.characters.pushBack(std::move(avatar));

  auto enemy = model::CharacterInstance{};
  enemy.id = "enemy-1";
  enemy.name = "Goblin";
  enemy.templateName = "goblinTest";
  enemy.x = enemyX;
  enemy.y = enemyY;
  enemy.currentHp = 12;
  enemy.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(enemy));

  for (size_t i = 0; i < state.world.activeMap.characters.size(); i++) {
    game::applyCharacterTemplateFromDatabase(state.world.activeMap.characters[i],
                                             database);
  }

  return state;
}

void lightFromAvatar(state::State& state, const db::Database& database) {
  auto* avatar = findOnActiveMap(state.world.activeMap, "ally-1");
  if (avatar == nullptr) {
    return;
  }
  game::updateActiveMapVisibilityFromPlayer(
      state.world, state.mapInstances, avatar->x, avatar->y, database);
}

int partyHpSum(const model::Player& player) {
  auto sum = 0;
  for (size_t i = 0; i < player.party.size(); i++) {
    sum += player.party[i].currentHp;
  }
  return sum;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestEnemyBehavior" << LOG_ENDL;
  auto ok = true;

  ok = assertEqual(game::chebyshevDistance(0, 0, 3, 1), 3, "chebyshev") && ok;
  ok = assertTrue(game::isChebyshevAdjacent(2, 2, 3, 2), "adjacent") && ok;
  ok = assertFalse(game::isChebyshevAdjacent(2, 2, 4, 2), "not adjacent") && ok;

  {
    db::Database database;
    auto state = makeTownState(database, 2, 2, 4, 2, 6);
    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    lightFromAvatar(stateManager.getState(), database);

    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr, "enemy exists") && ok;
    ok = assertTrue(
        game::canEnemySpotPartyAvatar(
            stateManager.getState().world,
            stateManager.getState().mapInstances,
            stateManager.getState().player,
            *enemy,
            database),
        "can spot in range + visible") &&
         ok;

    game::updateEnemySpotting(stateManager.getState().world,
                              stateManager.getState().mapInstances,
                              stateManager.getState().player,
                              database);
    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr && enemy->agitated, "agitated after spot") && ok;
  }

  {
    db::Database database;
    auto state = makeTownState(database, 2, 2, 10, 2, 6);
    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    lightFromAvatar(stateManager.getState(), database);

    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr, "enemy out of range exists") && ok;
    ok = assertFalse(
        game::canEnemySpotPartyAvatar(
            stateManager.getState().world,
            stateManager.getState().mapInstances,
            stateManager.getState().player,
            *enemy,
            database),
        "cannot spot out of vision radius") &&
         ok;

    game::updateEnemySpotting(stateManager.getState().world,
                              stateManager.getState().mapInstances,
                              stateManager.getState().player,
                              database);
    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr && !enemy->agitated, "not agitated out of range") &&
         ok;
  }

  {
    db::Database database;
    auto state = makeTownState(database, 2, 2, 4, 2, 6);
    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);

    // Visibility never updated → enemy tile not visible.
    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr, "enemy for not-visible") && ok;
    ok = assertFalse(
        game::canEnemySpotPartyAvatar(
            stateManager.getState().world,
            stateManager.getState().mapInstances,
            stateManager.getState().player,
            *enemy,
            database),
        "cannot spot when not visible") &&
         ok;
  }

  {
    db::Database database;
    auto state = makeTownState(database, 2, 2, 5, 2, 6);
    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    lightFromAvatar(stateManager.getState(), database);

    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr, "seek enemy exists") && ok;
    enemy->agitated = true;
    const auto startX = enemy->x;

    runAndPumpTownEnemyAi(stateManager, database);
    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr, "seek enemy after tick") && ok;
    if (enemy) {
      ok = assertEqual(enemy->x, startX - 1, "agitated enemy stepped west toward avatar") &&
           ok;
      ok = assertEqual(enemy->y, 2, "seek prefers cardinal toward avatar") && ok;
    }
    ok = assertFalse(stateManager.getState().world.resolvingTownEnemyAi,
                     "town AI idle after seek-only tick") &&
         ok;
  }

  {
    // Dist 2: one seek step enters melee, so the same town turn may also attack.
    db::Database database;
    auto state = makeTownState(database, 2, 2, 4, 2, 6);
    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    lightFromAvatar(stateManager.getState(), database);

    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr, "enter-melee enemy exists") && ok;
    enemy->agitated = true;
    const auto hpBefore = partyHpSum(stateManager.getState().player);

    std::srand(1);
    auto damaged = false;
    for (int attempt = 0; attempt < 40; ++attempt) {
      enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
      if (enemy) {
        // Reset to dist 2 so each attempt is step-into-melee + attack.
        enemy->x = 4;
        enemy->y = 2;
      }
      stateManager.getState().player.party[0].currentHp = 100;
      stateManager.getState().player.party[1].currentHp = 80;
      runAndPumpTownEnemyAi(stateManager, database);
      enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
      ok = assertTrue(enemy != nullptr && enemy->x == 3,
                      "enter-melee enemy stepped adjacent") &&
           ok;
      if (partyHpSum(stateManager.getState().player) < 180) {
        damaged = true;
        break;
      }
    }
    ok = assertTrue(damaged, "entering melee also attacks same town turn") && ok;
    (void)hpBefore;
  }

  {
    db::Database database;
    auto state = makeTownState(database, 2, 2, 3, 2, 6);
    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    lightFromAvatar(stateManager.getState(), database);

    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr, "melee enemy exists") && ok;
    enemy->agitated = true;
    const auto startX = enemy->x;
    const auto hpBefore = partyHpSum(stateManager.getState().player);

    // Force hits so the random 75% chance cannot flake the assertion.
    std::srand(1);
    auto damaged = false;
    for (int attempt = 0; attempt < 40; ++attempt) {
      runAndPumpTownEnemyAi(stateManager, database);
      if (partyHpSum(stateManager.getState().player) < hpBefore) {
        damaged = true;
        break;
      }
    }
    ok = assertTrue(damaged, "town melee eventually damages a party member") && ok;

    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemy != nullptr, "melee enemy still on map") && ok;
    if (enemy) {
      ok = assertEqual(enemy->x, startX, "adjacent enemy does not step away") && ok;
    }
    ok = assertFalse(stateManager.getState().world.resolvingTownEnemyAi,
                     "town AI idle after melee sequence") &&
         ok;

    // Non-avatar member can be the victim (random). Ensure ally-2 can take damage.
    stateManager.getState().player.party[0].currentHp = 100;
    stateManager.getState().player.party[1].currentHp = 80;
    auto ally2Hit = false;
    for (int attempt = 0; attempt < 80; ++attempt) {
      const auto ally2Before = stateManager.getState().player.party[1].currentHp;
      runAndPumpTownEnemyAi(stateManager, database);
      if (stateManager.getState().player.party[1].currentHp < ally2Before) {
        ally2Hit = true;
        break;
      }
    }
    ok = assertTrue(ally2Hit, "town melee can damage non-avatar party member") && ok;
  }

  {
    db::Database database;
    auto state = makeTownState(database, 2, 2, 6, 2, 6);
    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    lightFromAvatar(stateManager.getState(), database);

    // Player steps closer; spotting + seek should run via WorldMovePlayer.
    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::WorldMovePlayer(1, 0), 0);
    pumpTownEnemyAi(stateManager);

    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    auto* avatar = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    ok = assertTrue(enemy != nullptr && avatar != nullptr, "move+ai actors") && ok;
    if (enemy && avatar) {
      ok = assertTrue(enemy->agitated, "spotted after player move") && ok;
      ok = assertEqual(avatar->x, 3, "avatar moved east") && ok;
      ok = assertTrue(enemy->x < 6, "enemy sought after player move") && ok;
    }
    ok = assertFalse(stateManager.getState().world.resolvingTownEnemyAi,
                     "town AI idle after WorldMovePlayer") &&
         ok;
  }

  {
    db::Database database;
    addHeroTemplate(database);
    addGoblinTemplate(database, 6);
    state::DatabaseInterface::setDatabase(&database);

    state::State state;
    setupGrid(database, state, 5, 5);

    auto member = model::CharacterPlayer{};
    member.instanceId = "ally-1";
    member.name = "Hero";
    member.templateName = "hero";
    member.currentHp = 100;
    state.player.party.pushBack(std::move(member));

    auto ally = model::CharacterInstance{};
    ally.id = "ally-1";
    ally.name = "Hero";
    ally.templateName = "hero";
    ally.x = 1;
    ally.y = 2;
    state.world.activeMap.characters.pushBack(std::move(ally));

    auto enemy = model::CharacterInstance{};
    enemy.id = "enemy-1";
    enemy.name = "Goblin";
    enemy.templateName = "goblinTest";
    enemy.x = 4;
    enemy.y = 2;
    enemy.currentHp = 12;
    enemy.hpInitialized = true;
    state.world.activeMap.characters.pushBack(std::move(enemy));
    for (size_t i = 0; i < state.world.activeMap.characters.size(); i++) {
      game::applyCharacterTemplateFromDatabase(state.world.activeMap.characters[i],
                                               database);
    }

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);

    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::StartCombat(), 0);
    for (int i = 0; i < 20; ++i) {
      tickState(stateManager, 50);
    }

    auto& combat = stateManager.getState().world.combat;
    ok = assertTrue(combat.active, "combat started for cpu ai") && ok;

    // Force enemy turn.
    combat.activeCharacterId = "enemy-1";
    combat.isWaitingForAction = true;
    auto* enemyBefore = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    const auto enemyStartX = enemyBefore ? enemyBefore->x : -1;

    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::DoCPUCombatTurn(), 0);
    for (int i = 0; i < 40; ++i) {
      tickState(stateManager, 50);
    }

    auto* enemyAfter = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(enemyAfter != nullptr, "enemy after cpu turn") && ok;
    if (enemyAfter && enemyStartX >= 0) {
      ok = assertTrue(enemyAfter->x < enemyStartX, "cpu seek stepped toward party") && ok;
    }
  }

  {
    db::Database database;
    addHeroTemplate(database);
    addGoblinTemplate(database, 6);
    state::DatabaseInterface::setDatabase(&database);

    state::State state;
    setupGrid(database, state, 5, 5);

    auto member = model::CharacterPlayer{};
    member.instanceId = "ally-1";
    member.name = "Hero";
    member.templateName = "hero";
    member.currentHp = 100;
    state.player.party.pushBack(std::move(member));

    auto ally = model::CharacterInstance{};
    ally.id = "ally-1";
    ally.name = "Hero";
    ally.templateName = "hero";
    ally.x = 2;
    ally.y = 2;
    state.world.activeMap.characters.pushBack(std::move(ally));

    auto enemy = model::CharacterInstance{};
    enemy.id = "enemy-1";
    enemy.name = "Goblin";
    enemy.templateName = "goblinTest";
    enemy.x = 3;
    enemy.y = 2;
    enemy.currentHp = 12;
    enemy.hpInitialized = true;
    state.world.activeMap.characters.pushBack(std::move(enemy));
    for (size_t i = 0; i < state.world.activeMap.characters.size(); i++) {
      game::applyCharacterTemplateFromDatabase(state.world.activeMap.characters[i],
                                               database);
    }

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);

    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::StartCombat(), 0);
    for (int i = 0; i < 20; ++i) {
      tickState(stateManager, 50);
    }

    auto& combat = stateManager.getState().world.combat;
    combat.activeCharacterId = "enemy-1";
    auto* enemyAp = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    if (enemyAp) {
      enemyAp->currentAp = model::COMBAT_STARTING_AP;
    }
    combat.isWaitingForAction = true;

    const auto allyHpBefore = stateManager.getState().player.party[0].currentHp;
    std::srand(1);
    auto hit = false;
    for (int attempt = 0; attempt < 20; ++attempt) {
      combat.activeCharacterId = "enemy-1";
      enemyAp = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
      if (enemyAp) {
        enemyAp->currentAp = model::COMBAT_STARTING_AP;
      }
      combat.isWaitingForAction = false;
      stateManager.enqueueAction(
          stateManager.getActionData(), new state::actions::DoCPUCombatTurn(), 0);
      for (int i = 0; i < 40; ++i) {
        tickState(stateManager, 50);
      }
      if (stateManager.getState().player.party[0].currentHp < allyHpBefore) {
        hit = true;
        break;
      }
    }
    ok = assertTrue(hit, "cpu adjacent melee damages party") && ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestEnemyBehavior assertions failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestEnemyBehavior completed successfully" << LOG_ENDL;
  return 0;
}
