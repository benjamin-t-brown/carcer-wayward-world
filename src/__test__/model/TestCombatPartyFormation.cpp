#include "db/Database.h"
#include "game/combat/CombatParty.h"
#include "game/map/CharacterConstruction.h"
#include "game/map/MapWalkability.h"
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
#include "state/WorldUpdater.h"
#include "actions/combat/StartCombat.hpp"
#include "actions/world/WorldMovePlayer.hpp"
#include "actions/world/WorldWaitPlayer.hpp"
#include "bmin/String.h"

#define TEST_NAME "TestCombatPartyFormation"

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

void tickState(state::StateManager& stateManager, int dt) {
  stateManager.update(dt);
  state::worldUpdate(nullptr, stateManager, dt);
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
  auto floor = model::TileMetadata{};
  floor.id = 0;
  floor.isWalkable = true;
  floor.isSeeThrough = true;
  tileset.tiles.pushBack(floor);
  auto wall = model::TileMetadata{};
  wall.id = 1;
  wall.isWalkable = false;
  wall.isSeeThrough = false;
  tileset.tiles.pushBack(wall);
  database.addTilesetTemplate(tileset);
}

model::MapInstance makeEmptyMap(int width, int height) {
  auto map = model::MapInstance{};
  map.id = "formation_test_map";
  map.templateName = "formation_test_map";
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

void setWall(model::MapInstance& map, int x, int y) {
  auto* tile = game::tileAtCurrentLayer(map, x, y);
  if (tile) {
    tile->tileId = 1;
  }
}

void setupGrid(db::Database& database, state::State& state, int width, int height) {
  addTestTileset(database);
  auto map = makeEmptyMap(width, height);
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "formation_test_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = width;
  grid.mapHeight = height;
  grid.cells = {{"formation_test_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "formation_test_grid";
}

model::CharacterPlayer makePartyMember(const char* id, const char* name) {
  auto member = model::CharacterPlayer{};
  member.instanceId = id;
  member.name = name;
  member.templateName = "hero";
  member.currentHp = 100;
  return member;
}

void addHeroTemplate(db::Database& database) {
  auto allyTemplate = model::CharacterTemplate{};
  allyTemplate.type = model::CharacterTemplateType::TOWNSPERSON;
  allyTemplate.name = "hero";
  allyTemplate.combat.hp = 100;
  database.addCharacterTemplate(allyTemplate);
}

void addEnemyTemplate(db::Database& database) {
  auto enemyTemplate = model::CharacterTemplate{};
  enemyTemplate.type = model::CharacterTemplateType::ENEMY;
  enemyTemplate.name = "slime";
  enemyTemplate.combat.hp = 20;
  database.addCharacterTemplate(enemyTemplate);
}

state::State makeBaseState(db::Database& database, int width, int height) {
  addHeroTemplate(database);
  addEnemyTemplate(database);
  state::DatabaseInterface::setDatabase(&database);

  state::State state;
  setupGrid(database, state, width, height);
  state.world.camera.viewW = 100;
  state.world.camera.viewH = 80;
  return state;
}

void placeLeader(state::State& state, int x, int y, int lastDx, int lastDy) {
  state.player.party.pushBack(makePartyMember("ally-1", "Hero"));

  auto ally = model::CharacterInstance{};
  ally.id = "ally-1";
  ally.name = "Hero";
  ally.templateName = "hero";
  ally.x = x;
  ally.y = y;
  ally.lastMoveDx = lastDx;
  ally.lastMoveDy = lastDy;
  state.world.activeMap.characters.pushBack(std::move(ally));
}

void addMissingPartyMembers(state::State& state, int count) {
  // Leader is already ally-1; add count additional members not on the map.
  for (int i = 0; i < count; i++) {
    const auto index = i + 2;
    auto member = model::CharacterPlayer{};
    member.instanceId = bmin::String("ally-") + bmin::toString(index);
    member.name = bmin::String("Ally") + bmin::toString(index);
    member.templateName = "hero";
    member.currentHp = 100;
    state.player.party.pushBack(std::move(member));
  }
}

model::MapInstance& activeMapInstance(state::State& state) {
  return state.mapInstances["formation_test_map"];
}

bool positionsMatch(model::ActiveMap& activeMap,
                    const char* id,
                    int x,
                    int y,
                    const char* label) {
  auto* ch = findOnActiveMap(activeMap, id);
  if (!ch) {
    LOG(ERROR) << label << " missing character " << id << LOG_ENDL;
    return false;
  }
  return assertEqual(ch->x, x, label) && assertEqual(ch->y, y, label);
}

bool testOpenFormationShapes() {
  LOG(INFO) << "testOpenFormationShapes" << LOG_ENDL;
  auto ok = true;

  // Party of 2, facing north → line behind (south).
  {
    db::Database database;
    auto state = makeBaseState(database, 9, 9);
    placeLeader(state, 4, 2, 0, -1);
    addMissingPartyMembers(state, 1);

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
    tickState(stateManager, 1);

    auto& world = stateManager.getState().world;
    ok = positionsMatch(world.activeMap, "ally-1", 4, 2, "p2 leader") && ok;
    ok = positionsMatch(world.activeMap, "ally-2", 4, 3, "p2 ally behind") && ok;
  }

  // Party of 3, facing north → triangle.
  {
    db::Database database;
    auto state = makeBaseState(database, 9, 9);
    placeLeader(state, 4, 2, 0, -1);
    addMissingPartyMembers(state, 2);

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
    tickState(stateManager, 1);

    auto& world = stateManager.getState().world;
    ok = positionsMatch(world.activeMap, "ally-1", 4, 2, "p3 leader") && ok;
    ok = positionsMatch(world.activeMap, "ally-2", 3, 3, "p3 left") && ok;
    ok = positionsMatch(world.activeMap, "ally-3", 5, 3, "p3 right") && ok;
  }

  // Party of 4, facing north → triangle with centered third row.
  {
    db::Database database;
    auto state = makeBaseState(database, 9, 9);
    placeLeader(state, 4, 2, 0, -1);
    addMissingPartyMembers(state, 3);

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
    tickState(stateManager, 1);

    auto& world = stateManager.getState().world;
    ok = positionsMatch(world.activeMap, "ally-1", 4, 2, "p4 leader") && ok;
    ok = positionsMatch(world.activeMap, "ally-2", 3, 3, "p4 left") && ok;
    ok = positionsMatch(world.activeMap, "ally-3", 5, 3, "p4 right") && ok;
    ok = positionsMatch(world.activeMap, "ally-4", 4, 4, "p4 center back") && ok;
  }

  // Party of 3, facing east → fan west.
  {
    db::Database database;
    auto state = makeBaseState(database, 9, 9);
    placeLeader(state, 4, 4, 1, 0);
    addMissingPartyMembers(state, 2);

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
    tickState(stateManager, 1);

    auto& world = stateManager.getState().world;
    ok = positionsMatch(world.activeMap, "ally-1", 4, 4, "east leader") && ok;
    // behind = (-1,0), right = (0,1) → laterals -1 and +1 at depth 1
    ok = positionsMatch(world.activeMap, "ally-2", 3, 3, "east first") && ok;
    ok = positionsMatch(world.activeMap, "ally-3", 3, 5, "east second") && ok;
  }

  // Party of 3, facing NE → two adjacent rear cardinals (west + south).
  {
    db::Database database;
    auto state = makeBaseState(database, 9, 9);
    placeLeader(state, 4, 4, 1, -1);
    addMissingPartyMembers(state, 2);

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
    tickState(stateManager, 1);

    auto& world = stateManager.getState().world;
    ok = positionsMatch(world.activeMap, "ally-1", 4, 4, "ne3 leader") && ok;
    ok = positionsMatch(world.activeMap, "ally-2", 3, 4, "ne3 west") && ok;
    ok = positionsMatch(world.activeMap, "ally-3", 4, 5, "ne3 south") && ok;
  }

  // Party of 4, facing NE → adjacent pair, then centered ally pulled onto the
  // diagonal pocket between them.
  {
    db::Database database;
    auto state = makeBaseState(database, 9, 9);
    placeLeader(state, 4, 4, 1, -1);
    addMissingPartyMembers(state, 3);

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
    tickState(stateManager, 1);

    auto& world = stateManager.getState().world;
    ok = positionsMatch(world.activeMap, "ally-1", 4, 4, "ne4 leader") && ok;
    ok = positionsMatch(world.activeMap, "ally-2", 3, 4, "ne4 west") && ok;
    ok = positionsMatch(world.activeMap, "ally-3", 4, 5, "ne4 south") && ok;
    ok = positionsMatch(world.activeMap, "ally-4", 3, 5, "ne4 pocket") && ok;
  }

  // Party of 6, facing NE → row of three pulled one diagonal toward the leader.
  {
    db::Database database;
    auto state = makeBaseState(database, 9, 9);
    placeLeader(state, 4, 4, 1, -1);
    addMissingPartyMembers(state, 5);

    state::StateManager stateManager;
    stateManager.getState() = state;
    state::StateManagerInterface::setStateManager(&stateManager);
    stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
    tickState(stateManager, 1);

    auto& world = stateManager.getState().world;
    ok = positionsMatch(world.activeMap, "ally-1", 4, 4, "ne6 leader") && ok;
    ok = positionsMatch(world.activeMap, "ally-2", 3, 4, "ne6 west") && ok;
    ok = positionsMatch(world.activeMap, "ally-3", 4, 5, "ne6 south") && ok;
    ok = positionsMatch(world.activeMap, "ally-4", 2, 4, "ne6 row3 left") && ok;
    ok = positionsMatch(world.activeMap, "ally-5", 3, 5, "ne6 row3 mid") && ok;
    ok = positionsMatch(world.activeMap, "ally-6", 4, 6, "ne6 row3 right") && ok;
  }

  return ok;
}

bool testWaitPreservesDirection() {
  LOG(INFO) << "testWaitPreservesDirection" << LOG_ENDL;
  db::Database database;
  auto state = makeBaseState(database, 9, 9);
  placeLeader(state, 4, 4, 1, 0);
  addMissingPartyMembers(state, 1);

  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);

  // Move north, then wait; formation should still fan south.
  state::actions::WorldMovePlayer(0, -1).execute(&stateManager.getState());
  auto* leader = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
  auto ok = assertTrue(leader != nullptr, "leader after move") &&
            assertEqual(leader ? leader->lastMoveDx : 0, 0, "lastMoveDx after north") &&
            assertEqual(leader ? leader->lastMoveDy : 0, -1, "lastMoveDy after north");

  state::actions::WorldWaitPlayer().execute(&stateManager.getState());
  leader = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
  ok = assertTrue(leader != nullptr, "leader after wait") &&
       assertEqual(leader ? leader->lastMoveDx : 0, 0, "lastMoveDx after wait") &&
       assertEqual(leader ? leader->lastMoveDy : 0, -1, "lastMoveDy after wait") &&
       ok;

  stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
  tickState(stateManager, 1);

  auto& world = stateManager.getState().world;
  ok = positionsMatch(world.activeMap, "ally-1", 4, 3, "wait leader after north move") && ok;
  ok = positionsMatch(world.activeMap, "ally-2", 4, 4, "wait ally fans south") && ok;
  return ok;
}

bool testLongRangeFacingKeepsCompactFormation() {
  LOG(INFO) << "testLongRangeFacingKeepsCompactFormation" << LOG_ENDL;
  db::Database database;
  auto state = makeBaseState(database, 9, 9);
  // Flame-at-a-distance leftover: lastMove is the raw tile delta, not a unit step.
  placeLeader(state, 4, 4, 0, -5);
  addMissingPartyMembers(state, 1);

  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);
  stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
  tickState(stateManager, 1);

  auto& world = stateManager.getState().world;
  auto ok = positionsMatch(world.activeMap, "ally-1", 4, 4, "long-range leader");
  ok = positionsMatch(world.activeMap, "ally-2", 4, 5, "long-range ally one tile south") &&
       ok;
  return ok;
}

bool testLineFallbackWhenFormationBlocked() {
  LOG(INFO) << "testLineFallbackWhenFormationBlocked" << LOG_ENDL;
  db::Database database;
  auto state = makeBaseState(database, 9, 9);
  placeLeader(state, 4, 2, 0, -1);
  addMissingPartyMembers(state, 2);

  // Block triangle wings; leave the center line behind the leader open.
  auto& map = activeMapInstance(state);
  setWall(map, 3, 3);
  setWall(map, 5, 3);

  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);
  stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
  tickState(stateManager, 1);

  auto& world = stateManager.getState().world;
  auto ok = positionsMatch(world.activeMap, "ally-1", 4, 2, "line-fallback leader");
  ok = positionsMatch(world.activeMap, "ally-2", 4, 3, "line-fallback first") && ok;
  ok = positionsMatch(world.activeMap, "ally-3", 4, 4, "line-fallback second") && ok;
  return ok;
}

bool testStackFallbackWhenLineBlocked() {
  LOG(INFO) << "testStackFallbackWhenLineBlocked" << LOG_ENDL;
  db::Database database;
  auto state = makeBaseState(database, 9, 9);
  placeLeader(state, 4, 2, 0, -1);
  addMissingPartyMembers(state, 1);

  auto& map = activeMapInstance(state);
  setWall(map, 4, 3);

  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);
  stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
  tickState(stateManager, 1);

  auto& world = stateManager.getState().world;
  auto ok = positionsMatch(world.activeMap, "ally-1", 4, 2, "stack leader");
  ok = positionsMatch(world.activeMap, "ally-2", 4, 2, "stack ally on leader") && ok;
  return ok;
}

bool testEnemyBlocksFormationAndLine() {
  LOG(INFO) << "testEnemyBlocksFormationAndLine" << LOG_ENDL;
  db::Database database;
  auto state = makeBaseState(database, 9, 9);
  placeLeader(state, 4, 2, 0, -1);
  addMissingPartyMembers(state, 1);

  auto enemy = model::CharacterInstance{};
  enemy.id = "enemy-1";
  enemy.name = "Slime";
  enemy.templateName = "slime";
  enemy.x = 4;
  enemy.y = 3;
  enemy.currentHp = 20;
  state.world.activeMap.characters.pushBack(std::move(enemy));
  game::applyCharacterTemplateFromDatabase(
      *findOnActiveMap(state.world.activeMap, "enemy-1"), database);

  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);
  stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
  tickState(stateManager, 1);

  auto& world = stateManager.getState().world;
  auto ok = positionsMatch(world.activeMap, "ally-1", 4, 2, "enemy-block leader");
  ok = positionsMatch(world.activeMap, "ally-2", 4, 2, "enemy-block stack") && ok;
  return ok;
}

bool testNoTeleportOverWall() {
  LOG(INFO) << "testNoTeleportOverWall" << LOG_ENDL;
  db::Database database;
  auto state = makeBaseState(database, 9, 9);
  placeLeader(state, 4, 2, 0, -1);
  addMissingPartyMembers(state, 3);

  auto& map = activeMapInstance(state);
  // Wings (3,3)/(5,3) stay open for the first formation row. Seal every other
  // neighbor of (4,4) so depth-2 cannot be reached without teleporting after
  // the wings are occupied — and the rear line is blocked by (4,3).
  setWall(map, 4, 3);
  setWall(map, 3, 4);
  setWall(map, 5, 4);
  setWall(map, 3, 5);
  setWall(map, 4, 5);
  setWall(map, 5, 5);

  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);
  stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
  tickState(stateManager, 1);

  auto& world = stateManager.getState().world;
  auto* deep = findOnActiveMap(world.activeMap, "ally-4");
  auto ok = assertTrue(deep != nullptr, "deep ally exists");
  if (deep) {
    ok = assertTrue(!(deep->x == 4 && deep->y == 4),
                    "deep ally must not teleport to unreachable (4,4)") &&
         ok;
  }

  // Formation fails at depth-2; line blocked by wall at (4,3) → stack.
  ok = positionsMatch(world.activeMap, "ally-2", 4, 2, "no-teleport ally2 stack") && ok;
  ok = positionsMatch(world.activeMap, "ally-3", 4, 2, "no-teleport ally3 stack") && ok;
  ok = positionsMatch(world.activeMap, "ally-4", 4, 2, "no-teleport ally4 stack") && ok;
  return ok;
}

bool facingMatches(model::ActiveMap& activeMap,
                   const char* id,
                   model::CharacterFacing facing,
                   int lastDx,
                   int lastDy,
                   const char* label) {
  auto* ch = findOnActiveMap(activeMap, id);
  if (!ch) {
    LOG(ERROR) << label << " missing character " << id << LOG_ENDL;
    return false;
  }
  return assertTrue(ch->facing == facing, label) &&
         assertEqual(ch->lastMoveDx, lastDx, label) &&
         assertEqual(ch->lastMoveDy, lastDy, label);
}

bool testPartyFacesLeaderOnCombatStart() {
  LOG(INFO) << "testPartyFacesLeaderOnCombatStart" << LOG_ENDL;
  db::Database database;
  auto state = makeBaseState(database, 9, 9);
  // Leader faces west. One ally already on the map faces the other way.
  placeLeader(state, 4, 4, -1, 0);
  addMissingPartyMembers(state, 2);

  auto allyOnMap = model::CharacterInstance{};
  allyOnMap.id = "ally-2";
  allyOnMap.name = "Ally2";
  allyOnMap.templateName = "hero";
  allyOnMap.x = 5;
  allyOnMap.y = 4;
  allyOnMap.facing = model::CharacterFacing::Right;
  allyOnMap.lastMoveDx = 1;
  allyOnMap.lastMoveDy = 0;
  state.world.activeMap.characters.pushBack(std::move(allyOnMap));

  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);
  stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
  tickState(stateManager, 1);

  auto& world = stateManager.getState().world;
  auto ok =
      facingMatches(world.activeMap,
                    "ally-1",
                    model::CharacterFacing::Left,
                    -1,
                    0,
                    "leader keeps west facing");
  ok = facingMatches(world.activeMap,
                     "ally-2",
                     model::CharacterFacing::Left,
                     -1,
                     0,
                     "on-map ally matches leader facing") &&
       ok;
  ok = facingMatches(world.activeMap,
                     "ally-3",
                     model::CharacterFacing::Left,
                     -1,
                     0,
                     "spawned ally matches leader facing") &&
       ok;
  return ok;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;
  auto ok = true;
  ok = testOpenFormationShapes() && ok;
  ok = testWaitPreservesDirection() && ok;
  ok = testLongRangeFacingKeepsCompactFormation() && ok;
  ok = testLineFallbackWhenFormationBlocked() && ok;
  ok = testStackFallbackWhenLineBlocked() && ok;
  ok = testEnemyBlocksFormationAndLine() && ok;
  ok = testNoTeleportOverWall() && ok;
  ok = testPartyFacesLeaderOnCombatStart() && ok;

  if (ok) {
    LOG(INFO) << TEST_NAME << " PASSED" << LOG_ENDL;
    return 0;
  }
  LOG(ERROR) << TEST_NAME << " FAILED" << LOG_ENDL;
  return 1;
}
