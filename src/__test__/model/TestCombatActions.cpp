#include "db/Database.h"
#include "game/map/MapPersistence.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/MapInstance.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/MapGrids.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/actions/combat/DoCombatAction.hpp"
#include "state/actions/combat/EndCombat.hpp"
#include "state/actions/combat/ModifyAP.hpp"
#include "state/actions/combat/ModifyHP.hpp"
#include "state/actions/combat/StartCombat.hpp"
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

model::CharacterInstance* findOnActiveMap(model::ActiveMap& activeMap,
                                          const bmin::String& id) {
  for (auto& ch : activeMap.characters) {
    if (ch.id == id) {
      return &ch;
    }
  }
  return nullptr;
}

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  auto meta = model::TileMetadata{};
  meta.id = 0;
  meta.isWalkable = true;
  meta.isSeeThrough = true;
  tileset.tiles.pushBack(meta);
  database.addTilesetTemplate(tileset);
}

model::MapInstance makeEmptyMap(int width, int height) {
  auto map = model::MapInstance{};
  map.id = "combat_test_map";
  map.templateName = "combat_test_map";
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
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  return map;
}

void setupCombatGrid(db::Database& database, state::State& state) {
  addTestTileset(database);
  auto map = makeEmptyMap(5, 5);
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "combat_test_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 5;
  grid.mapHeight = 5;
  grid.cells = {{"combat_test_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "combat_test_grid";
}

state::State makeCombatState(db::Database& database) {
  auto allyTemplate = model::CharacterTemplate{};
  allyTemplate.type = model::CharacterTemplateType::TOWNSPERSON;
  allyTemplate.name = "hero";
  allyTemplate.combat.hp = 100;
  database.addCharacterTemplate(allyTemplate);

  auto enemyTemplate = model::CharacterTemplate{};
  enemyTemplate.type = model::CharacterTemplateType::ENEMY;
  enemyTemplate.name = "slime";
  enemyTemplate.combat.hp = 20;
  database.addCharacterTemplate(enemyTemplate);

  state::DatabaseInterface::setDatabase(&database);

  state::State state;
  setupCombatGrid(database, state);
  state.world.camera.viewW = 100;
  state.world.camera.viewH = 80;

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
  enemy.name = "Slime";
  enemy.templateName = "slime";
  enemy.x = 3;
  enemy.y = 2;
  enemy.currentHp = 20;
  state.world.activeMap.characters.pushBack(std::move(enemy));

  for (size_t i = 0; i < state.world.activeMap.characters.size(); i++) {
    model::tryApplyCharacterTemplateToInstance(state.world.activeMap.characters[i],
                                               database);
  }

  return state;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestCombatActions" << LOG_ENDL;
  auto ok = true;

  db::Database database;
  auto state = makeCombatState(database);
  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);

  {
    stateManager.enqueueAction(stateManager.getActionData(),
                               new state::actions::StartCombat(),
                               0);
    stateManager.update(1);

    auto& world = stateManager.getState().world;
    ok = assertTrue(world.combat.active, "combat.active") && ok;
    ok = assertEqual(static_cast<int>(world.combat.turnOrderIds.size()), 2,
                     "turn order size") &&
         ok;
    ok = assertEqual(static_cast<int>(stateManager.getState().turnMode),
                     static_cast<int>(model::TurnMode::TURN_COMBAT),
                     "turn mode") &&
         ok;

    auto* ally = findOnActiveMap(world.activeMap, "ally-1");
    ok = assertTrue(ally != nullptr, "ally on map") && ok;
    if (ally) {
      ok = assertEqual(ally->currentAp, model::COMBAT_STARTING_AP, "ally starting AP") &&
           ok;
    }
  }

  {
    state::actions::ModifyAP("ally-1", -2).execute(&stateManager.getState());
    state::actions::ModifyHP("enemy-1", -5).execute(&stateManager.getState());

    auto* ally = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(ally != nullptr && enemy != nullptr, "combatants exist") && ok;
    if (ally) {
      ok = assertEqual(ally->currentAp, model::COMBAT_STARTING_AP - 2, "ally AP after -2") &&
           ok;
    }
    if (enemy) {
      ok = assertEqual(enemy->currentHp, 15, "enemy HP after -5") && ok;
    }
  }

  {
    auto* allyBefore = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    const auto startX = allyBefore ? allyBefore->x : -1;
    stateManager.enqueueAction(
        stateManager.getActionData(),
        new state::actions::DoCombatAction(model::CombatActionType::WAIT),
        0);
    for (int i = 0; i < 30; ++i) {
      stateManager.update(50);
    }
    auto* ally = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    ok = assertTrue(ally != nullptr, "ally after wait") && ok;
    if (ally && startX >= 0) {
      ok = assertEqual(ally->x, startX, "ally.x unchanged after wait") && ok;
    }
  }

  {
    stateManager.enqueueAction(stateManager.getActionData(),
                               new state::actions::EndCombat(),
                               0);
    stateManager.update(1);
    ok = assertTrue(!stateManager.getState().world.combat.active, "combat ended") && ok;
    ok = assertEqual(static_cast<int>(stateManager.getState().turnMode),
                     static_cast<int>(model::TurnMode::TURN_TOWN),
                     "town mode after end") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestCombatActions assertions failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestCombatActions completed successfully" << LOG_ENDL;
  return 0;
}
