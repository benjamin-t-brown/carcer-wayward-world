#include "db/Database.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/World.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/actions/combat/EndCombat.hpp"
#include "state/actions/combat/ModifyAP.hpp"
#include "state/actions/combat/ModifyHP.hpp"
#include "state/actions/combat/StartCombat.hpp"
#include "state/actions/combat/SetActiveCombatCharacter.hpp"

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

bool assertEqualStr(const bmin::String& actual, const bmin::String& expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

model::TileMetadata makeMeta(int id, bool walkable) {
  auto meta = model::TileMetadata{};
  meta.id = id;
  meta.isWalkable = walkable;
  meta.isSeeThrough = walkable;
  return meta;
}

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  tileset.tiles.pushBack(makeMeta(0, true));
  database.addTilesetTemplate(tileset);
}

model::MapInstance makeEmptyMap(int width, int height) {
  auto map = model::MapInstance{};
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
  model::mapLayerAt(map.tiles, 0) = std::move(layer);
  return map;
}

void addAllyTemplate(db::Database& database) {
  auto character = model::CharacterTemplate{};
  character.type = model::CharacterTemplateType::TOWNSPERSON;
  character.name = "hero";
  character.combat.hp = 100;
  database.addCharacterTemplate(character);
}

void addEnemyTemplate(db::Database& database) {
  auto character = model::CharacterTemplate{};
  character.type = model::CharacterTemplateType::ENEMY;
  character.name = "slime";
  character.combat.hp = 20;
  database.addCharacterTemplate(character);
}

state::State makeCombatState(db::Database& database) {
  addTestTileset(database);
  addAllyTemplate(database);
  addEnemyTemplate(database);
  state::DatabaseInterface::setDatabase(&database);

  state::State state;
  state.world.currentMap = makeEmptyMap(5, 5);
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
  state.world.currentMap.characters.pushBack(std::move(ally));

  auto enemy = model::CharacterInstance{};
  enemy.id = "enemy-1";
  enemy.name = "Slime";
  enemy.templateName = "slime";
  enemy.x = 3;
  enemy.y = 2;
  enemy.currentHp = 20;
  state.world.currentMap.characters.pushBack(std::move(enemy));

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

  // startCombat
  {
    stateManager.enqueueAction(stateManager.getActionData(),
                               new state::actions::StartCombat(),
                               0);
    stateManager.update(1);

    auto& world = stateManager.getState().world;
    ok = assertTrue(world.combat.active, "combat.active") && ok;
    ok = assertEqual(static_cast<int>(world.combat.turnOrderIds.size()), 2, "turn order size") &&
         ok;
    ok = assertEqual(static_cast<int>(world.currentMap.turnMode),
                     static_cast<int>(model::TurnMode::TURN_COMBAT),
                     "turn mode") &&
         ok;

    auto* ally = model::findCharacterOnMap(world.currentMap, "ally-1");
    ok = assertTrue(ally != nullptr, "ally on map") && ok;
    if (ally) {
      ok = assertEqual(ally->currentAp, model::COMBAT_STARTING_AP, "ally starting AP") && ok;
    }
  }

  // modifyAP / modifyHP
  {
    state::actions::ModifyAP("ally-1", -2).execute(&stateManager.getState());
    state::actions::ModifyHP("enemy-1", -5).execute(&stateManager.getState());

    auto* ally = model::findCharacterOnMap(stateManager.getState().world.currentMap, "ally-1");
    auto* enemy = model::findCharacterOnMap(stateManager.getState().world.currentMap, "enemy-1");
    ok = assertTrue(ally != nullptr && enemy != nullptr, "combatants exist") && ok;
    if (ally) {
      ok = assertEqual(ally->currentAp, model::COMBAT_STARTING_AP - 2, "modifyAP") && ok;
    }
    if (enemy) {
      ok = assertEqual(enemy->currentHp, 15, "modifyHP") && ok;
    }
  }

  // CPU enemy turn waits and advances back to ally
  {
    auto& world = stateManager.getState().world;
    world.combat.activeTurnIndex = 1;
    world.combat.activeCharacterId = "enemy-1";

    stateManager.enqueueAction(stateManager.getActionData(),
                               new state::actions::SetActiveCombatCharacter("enemy-1"),
                               0);
    stateManager.update(1);
    stateManager.update(1);

    ok = assertEqual(world.combat.activeTurnIndex, 0, "turn advanced after enemy wait") && ok;
    ok = assertEqualStr(world.combat.activeCharacterId, "ally-1", "back to ally") && ok;
  }

  // endCombat removes extra party members and clears combat
  {
    auto& world = stateManager.getState().world;
    auto member2 = model::CharacterPlayer{};
    member2.instanceId = "ally-2";
    member2.name = "Sidekick";
    member2.templateName = "hero";
    stateManager.getState().player.party.pushBack(std::move(member2));

    auto extra = model::CharacterInstance{};
    extra.id = "ally-2";
    extra.templateName = "hero";
    extra.x = 2;
    extra.y = 2;
    world.currentMap.characters.pushBack(std::move(extra));

    state::actions::EndCombat().execute(&stateManager.getState());
    ok = assertFalse(world.combat.active, "combat ended") && ok;

    auto foundExtra = false;
    for (const auto& character : world.currentMap.characters) {
      if (character.id == "ally-2") {
        foundExtra = true;
      }
    }
    ok = assertFalse(foundExtra, "extra party member removed") && ok;
  }

  if (ok) {
    LOG(INFO) << "TestCombatActions PASSED" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << "TestCombatActions FAILED" << LOG_ENDL;
  return 1;
}
