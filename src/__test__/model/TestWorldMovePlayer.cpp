#include "db/Database.h"
#include "game/map/Camera.h"
#include "game/map/MapPersistence.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileTriggers.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/MapInstance.h"
#include "model/templates/Items.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/Maps.h"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/WorldUpdater.h"
#include "actions/world/WorldMovePlayer.hpp"
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

bool assertFalse(bool cond, const char* label) {
  if (cond) {
    LOG(ERROR) << label << " expected false" << LOG_ENDL;
    return false;
  }
  return true;
}

model::TileMetadata makeMeta(int id, bool walkable, bool isDoor) {
  auto meta = model::TileMetadata{};
  meta.id = id;
  meta.isWalkable = walkable;
  meta.isDoor = isDoor;
  meta.isSeeThrough = walkable;
  return meta;
}

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  // 0 floor walkable, 1 wall, 2 closed door, 3 open door
  tileset.tiles.pushBack(makeMeta(0, true, false));
  tileset.tiles.pushBack(makeMeta(1, false, false));
  tileset.tiles.pushBack(makeMeta(2, false, true));
  tileset.tiles.pushBack(makeMeta(3, true, true));
  database.addTilesetTemplate(tileset);
}

model::TileInstance makeTile(int x, int y, int tileId) {
  auto tile = model::TileInstance{};
  tile.x = x;
  tile.y = y;
  tile.tilesetName = "test_terrain";
  tile.tileId = tileId;
  return tile;
}

bmin::DynArray<model::TileInstance> makeLayerTiles(int width, int height, int tileId) {
  auto layerTiles = bmin::DynArray<model::TileInstance>{};
  layerTiles.reserve(static_cast<size_t>(width * height));
  for (auto y = 0; y < height; y++) {
    for (auto x = 0; x < width; x++) {
      layerTiles.pushBack(makeTile(x, y, tileId));
    }
  }
  return layerTiles;
}

model::MapInstance makeEmptyMap(int width, int height) {
  auto map = model::MapInstance{};
  map.id = "test_map";
  map.templateName = "test_map";
  map.width = width;
  map.height = height;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  map.tileLayerNumber = 0;
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = makeLayerTiles(width, height, 0);
  return map;
}

model::TileInstance* tileAt(model::MapInstance& map, int x, int y, int layer = 0) {
  auto index = y * map.width + x;
  return &model::mapLayerAt(model::mapInstanceTiles(map), layer)[static_cast<size_t>(index)];
}

void setupGrid(db::Database& database, state::State& state, int width, int height) {
  model::MapGridTemplate grid;
  grid.name = "test_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = width;
  grid.mapHeight = height;
  grid.cells = {{"test_map"}};
  database.addMapGridTemplate(grid);

  auto map = makeEmptyMap(width, height);
  state.mapInstances[map.templateName] = std::move(map);
  state.world.activeMap.gridId = "test_grid";
  state.world.activeMap.mapLayer = 0;
}

model::CharacterInstance* spawnAvatar(state::State& state, int x, int y) {
  auto member = model::CharacterPlayer{};
  member.instanceId = "party-avatar";
  member.name = "Hero";
  member.templateName = "testPartyMember1";
  state.player.party.pushBack(std::move(member));
  state.player.currentPartyMemberIndex = 0;

  auto avatar = model::CharacterInstance{};
  avatar.id = "party-avatar";
  avatar.name = "Hero";
  avatar.templateName = "testPartyMember1";
  avatar.x = x;
  avatar.y = y;
  state.world.activeMap.characters.pushBack(std::move(avatar));
  return game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
}

void move(state::State& state, int dx, int dy) {
  auto action = state::actions::WorldMovePlayer(dx, dy);
  action.execute(&state);
}

model::MapInstance& mapOf(state::State& state) {
  return state.mapInstances["test_map"];
}

void addUtilityItem(db::Database& database, const char* name, bool isLockTool) {
  auto item = model::ItemTemplate{};
  item.name = name;
  item.itemType = model::ItemType::UTILITY;
  item.stackable = true;
  item.isLockTool = isLockTool;
  database.addItemTemplate(item);
}

void giveStack(model::CharacterPlayer& leader,
               const char* name,
               int quantity,
               const char* id) {
  auto stack = model::CharacterInventoryItem{};
  stack.itemName = name;
  stack.id = id;
  stack.quantity = quantity;
  leader.inventory.pushBack(std::move(stack));
}

int itemQuantity(const model::CharacterPlayer& leader, const char* name) {
  auto total = int{0};
  for (size_t i = 0; i < leader.inventory.size(); i++) {
    if (leader.inventory[i].itemName == name) {
      total += leader.inventory[i].quantity;
    }
  }
  return total;
}

int stackQuantityById(const model::CharacterPlayer& leader, const char* id) {
  for (size_t i = 0; i < leader.inventory.size(); i++) {
    if (leader.inventory[i].id == id) {
      return leader.inventory[i].quantity;
    }
  }
  return -1;
}

bool soundsContain(const state::State& state, const char* name) {
  for (size_t i = 0; i < state.soundsToPlay.size(); i++) {
    if (state.soundsToPlay[i] == name) {
      return true;
    }
  }
  return false;
}

void setLock(model::TileInstance& door, int lockLevel, const char* keyItem) {
  auto lock = model::DoorLock{};
  lock.lockLevel = lockLevel;
  lock.keyItem = keyItem;
  door.doorLock = lock;
}

model::CarcerMapTemplate makeClosedDoorTemplate(int doorX, int doorY) {
  auto tmpl = model::CarcerMapTemplate{};
  tmpl.name = "rebuild_door_map";
  tmpl.width = 5;
  tmpl.height = 5;
  tmpl.tilesets.pushBack("test_terrain");
  tmpl.layers.pushBack(0);
  auto graphics = bmin::DynArray<int>{};
  graphics.reserve(static_cast<size_t>(5 * 5 * 2));
  for (auto y = 0; y < 5; y++) {
    for (auto x = 0; x < 5; x++) {
      graphics.pushBack(0);
      graphics.pushBack(x == doorX && y == doorY ? 2 : 0);
    }
  }
  tmpl.tiles.pushBack(std::move(graphics));
  return tmpl;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldMovePlayer" << LOG_ENDL;

  bool ok = true;

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    addTestTileset(database);
    addUtilityItem(database, "ShedKey", false);
    addUtilityItem(database, "Lockpicks", true);
    addUtilityItem(database, "Twine", false);

    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);

    // Pure helper: override true vs non-walkable tileset
    {
      auto tile = makeTile(0, 0, 1);
      ok = assertFalse(game::isTileEffectivelyWalkable(tile, database),
                       "wall without override walkable") &&
           ok;
      tile.tileOverrides = model::TileOverrides{};
      tile.tileOverrides->isWalkableOverride = true;
      ok = assertTrue(game::isTileEffectivelyWalkable(tile, database),
                      "override true wins over wall") &&
           ok;
    }

    // Pure helper: override false vs walkable tileset
    {
      auto tile = makeTile(0, 0, 0);
      ok = assertTrue(game::isTileEffectivelyWalkable(tile, database),
                      "floor without override walkable") &&
           ok;
      tile.tileOverrides = model::TileOverrides{};
      tile.tileOverrides->isWalkableOverride = false;
      ok = assertFalse(game::isTileEffectivelyWalkable(tile, database),
                       "override false wins over floor") &&
           ok;
    }

    // Walk onto walkable tile
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "walk right.x") && ok;
      ok = assertEqual(avatar->y, 2, "walk right.y") && ok;
      ok = assertTrue(avatar->facing == model::CharacterFacing::Right, "walk right facing") &&
           ok;
    }

    // Facing updates on world move
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, -1, 0);
      ok = assertTrue(avatar->facing == model::CharacterFacing::Left, "walk left facing") && ok;
      state.world.resolvingTownEnemyAi = false;
      move(state, 0, 1);
      ok = assertTrue(avatar->facing == model::CharacterFacing::Left, "walk down facing") && ok;
      state.world.resolvingTownEnemyAi = false;
      move(state, 1, -1);
      ok = assertTrue(avatar->facing == model::CharacterFacing::Right, "walk up-right facing") &&
           ok;
    }

    // Blocked move still updates facing
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      tileAt(mapOf(state), 3, 2)->tileId = 1;
      auto* avatar = spawnAvatar(state, 2, 2);
      avatar->facing = model::CharacterFacing::Left;
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "blocked move.x") && ok;
      ok = assertTrue(avatar->facing == model::CharacterFacing::Right,
                      "blocked move updates facing") &&
           ok;
    }

    // Blocked by non-walkable non-door
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      tileAt(mapOf(state), 3, 2)->tileId = 1;
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "blocked wall.x") && ok;
      ok = assertEqual(avatar->y, 2, "blocked wall.y") && ok;
    }

    // Override true allows move onto tileset-non-walkable
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      auto* dest = tileAt(mapOf(state), 3, 2);
      dest->tileId = 1;
      dest->tileOverrides = model::TileOverrides{};
      dest->tileOverrides->isWalkableOverride = true;
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "override true move.x") && ok;
      ok = assertEqual(avatar->y, 2, "override true move.y") && ok;
    }

    // Override false blocks tileset-walkable
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      auto* dest = tileAt(mapOf(state), 3, 2);
      dest->tileId = 0;
      dest->tileOverrides = model::TileOverrides{};
      dest->tileOverrides->isWalkableOverride = false;
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "override false block.x") && ok;
      ok = assertEqual(avatar->y, 2, "override false block.y") && ok;
    }

    // Empty TileOverrides object (no authored isWalkableOverride) must not block
    {
      auto tile = makeTile(0, 0, 0);
      tile.tileOverrides = model::TileOverrides{};
      ok = assertTrue(game::isTileEffectivelyWalkable(tile, database),
                      "empty overrides object still walkable") &&
           ok;
    }

    // Current layer only: wall on another layer must not block walkable current layer
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      mapOf(state).tileLayerNumber = 0;
      state.world.activeMap.mapLayer = 0;
      model::mapLayerAt(model::mapInstanceTiles(mapOf(state)), 1) =
          makeLayerTiles(5, 5, 1);
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "other-layer wall ignored.x") && ok;
      ok = assertEqual(avatar->y, 2, "other-layer wall ignored.y") && ok;
    }

    // Current layer only: wall on tileLayerNumber blocks even if other layers are floor
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      model::mapLayerAt(model::mapInstanceTiles(mapOf(state)), 1) =
          makeLayerTiles(5, 5, 0);
      tileAt(mapOf(state), 3, 2, 0)->tileId = 1;
      mapOf(state).tileLayerNumber = 0;
      state.world.activeMap.mapLayer = 0;
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "current-layer wall blocks.x") && ok;
      ok = assertEqual(avatar->y, 2, "current-layer wall blocks.y") && ok;
    }

    // Current layer only: pointing mapLayer at a wall layer blocks
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      model::mapLayerAt(model::mapInstanceTiles(mapOf(state)), 1) =
          makeLayerTiles(5, 5, 1);
      state.world.activeMap.mapLayer = 1;
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "tileLayerNumber wall blocks.x") && ok;
      ok = assertEqual(avatar->y, 2, "tileLayerNumber wall blocks.y") && ok;
    }

    // Closed door: open without moving, then walk through
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "door bump.x") && ok;
      ok = assertEqual(avatar->y, 2, "door bump.y") && ok;
      ok = assertEqual(door->tileId, 3, "door opened tileId") && ok;

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "walk through open door.x") && ok;
      ok = assertEqual(avatar->y, 2, "walk through open door.y") && ok;
      ok = assertEqual(door->tileId, 3, "open door tileId unchanged after walk") && ok;
    }

    // Already-open door: move onto it, tileId unchanged
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 3;
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "already-open door move.x") && ok;
      ok = assertEqual(avatar->y, 2, "already-open door move.y") && ok;
      ok = assertEqual(door->tileId, 3, "already-open door tileId unchanged") && ok;
    }

    // Closed door with walkable override still opens
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      door->tileOverrides = model::TileOverrides{};
      door->tileOverrides->isWalkableOverride = true;
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "closed+override still opens.x") && ok;
      ok = assertEqual(avatar->y, 2, "closed+override still opens.y") && ok;
      ok = assertEqual(door->tileId, 3, "closed+override opened tileId") && ok;
    }

    // Open door with non-walkable override: move blocked
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 3;
      door->tileOverrides = model::TileOverrides{};
      door->tileOverrides->isWalkableOverride = false;
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "open+override false no move.x") && ok;
      ok = assertEqual(avatar->y, 2, "open+override false no move.y") && ok;
      ok = assertEqual(door->tileId, 3, "open+override false tileId unchanged") && ok;
    }

    // Door open mutates only the current layer
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      model::mapLayerAt(model::mapInstanceTiles(mapOf(state)), 1) =
          makeLayerTiles(5, 5, 0);
      auto* doorLayer0 = tileAt(mapOf(state), 3, 2, 0);
      auto* doorLayer1 = tileAt(mapOf(state), 3, 2, 1);
      doorLayer0->tileId = 2;
      doorLayer1->tileId = 2;
      state.world.activeMap.mapLayer = 0;
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "current-layer door bump.x") && ok;
      ok = assertEqual(doorLayer0->tileId, 3, "current-layer door opened") && ok;
      ok = assertEqual(doorLayer1->tileId, 2, "other-layer door unchanged") && ok;
    }

    // Out of bounds no-op
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      auto* avatar = spawnAvatar(state, 0, 0);
      move(state, -1, 0);
      ok = assertEqual(avatar->x, 0, "oob left.x") && ok;
      ok = assertEqual(avatar->y, 0, "oob left.y") && ok;
      move(state, 0, -1);
      ok = assertEqual(avatar->x, 0, "oob up.x") && ok;
      ok = assertEqual(avatar->y, 0, "oob up.y") && ok;
    }

    // Occupied by another character: blocked (town/outdoor), facing still updates
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      spawnAvatar(state, 2, 2);

      auto npc = model::CharacterInstance{};
      npc.id = "town-npc";
      npc.name = "Townsfolk";
      npc.templateName = "testNpc";
      npc.x = 3;
      npc.y = 2;
      state.world.activeMap.characters.pushBack(std::move(npc));

      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
      avatar->facing = model::CharacterFacing::Left;

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "occupied tile block.x") && ok;
      ok = assertEqual(avatar->y, 2, "occupied tile block.y") && ok;
      ok = assertTrue(avatar->facing == model::CharacterFacing::Right,
                      "occupied tile block updates facing") &&
           ok;
    }

    // Camera follow recenters after successful move
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 10, 10);
      state.world.camera.viewW = 100;
      state.world.camera.viewH = 80;
      state.world.camera.cameraMode = model::CameraMode::Follow;
      auto* avatar = spawnAvatar(state, 2, 2);
      state.world.camera.cameraFollowCharacterId = avatar->id;

      state::worldUpdate(nullptr, stateManager, 16);
      auto before = game::computeCameraFollow(
          2, 2, state.world.camera.viewW, state.world.camera.viewH);
      ok = assertEqual(state.world.camera.camX, before.camX, "cam before.x") && ok;
      ok = assertEqual(state.world.camera.camY, before.camY, "cam before.y") && ok;

      move(state, 1, 0);
      state::worldUpdate(nullptr, stateManager, 16);
      auto after = game::computeCameraFollow(
          3, 2, state.world.camera.viewW, state.world.camera.viewH);
      ok = assertEqual(avatar->x, 3, "cam follow move.x") && ok;
      ok = assertEqual(state.world.camera.camX, after.camX, "cam after.x") && ok;
      ok = assertEqual(state.world.camera.camY, after.camY, "cam after.y") && ok;
    }

    // Unlocked door still opens, consumes nothing, then the next bump walks through
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      giveStack(leader, "Lockpicks", 4, "picks");
      giveStack(leader, "ShedKey", 1, "key");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "unlocked door bump.x") && ok;
      ok = assertEqual(avatar->y, 2, "unlocked door bump.y") && ok;
      ok = assertEqual(door->tileId, 3, "unlocked door opened tileId") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 4, "unlocked door picks unchanged") &&
           ok;
      ok = assertEqual(itemQuantity(leader, "ShedKey"), 1, "unlocked door key unchanged") && ok;
      ok = assertFalse(soundsContain(state, "lockpick"), "unlocked door stays silent") && ok;
      ok = assertFalse(soundsContain(state, "hit_punch1"), "unlocked door is not a bash") && ok;
      const auto& doors = mapOf(state).persistentState.openedDoors;
      ok = assertEqual(static_cast<int>(doors.size()), 1, "unlocked door recorded") && ok;
      if (!doors.empty()) {
        ok = assertEqual(doors[0].layer, 0, "unlocked door record.layer") && ok;
        ok = assertEqual(doors[0].x, 3, "unlocked door record.x") && ok;
        ok = assertEqual(doors[0].y, 2, "unlocked door record.y") && ok;
        ok = assertEqual(doors[0].tileId, 3, "unlocked door record.tileId") && ok;
      }

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "unlocked door walk through.x") && ok;
      ok = assertEqual(door->tileId, 3, "unlocked door stays open after walk") && ok;
      ok = assertEqual(static_cast<int>(mapOf(state).persistentState.openedDoors.size()),
                       1,
                       "walk through does not add another door record") &&
           ok;
    }

    // Level 0 opens when the leader holds the named key, and the key is not consumed
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 0, "ShedKey");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.brutishness = 25;
      leader.stats.skills.trickery = 10;
      giveStack(leader, "ShedKey", 2, "key");
      giveStack(leader, "Lockpicks", 6, "picks");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "key door bump.x") && ok;
      ok = assertEqual(avatar->y, 2, "key door bump.y") && ok;
      ok = assertEqual(door->tileId, 3, "key door opened tileId") && ok;
      ok = assertEqual(itemQuantity(leader, "ShedKey"), 2, "key quantity unchanged") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 6, "key door does not spend tools") &&
           ok;
      ok = assertTrue(soundsContain(state, "lockpick"), "key door plays lockpick") && ok;
      ok = assertEqual(static_cast<int>(mapOf(state).persistentState.openedDoors.size()),
                       1,
                       "key door recorded") &&
           ok;
      if (!mapOf(state).persistentState.openedDoors.empty()) {
        ok = assertEqual(mapOf(state).persistentState.openedDoors[0].tileId,
                         3,
                         "key door record is open tile") &&
             ok;
      }

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "key door walk through.x") && ok;
      ok = assertEqual(itemQuantity(leader, "ShedKey"), 2, "key still held after walk") && ok;
    }

    // Level 0 without the named key stays closed and does not record a door
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 0, "ShedKey");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.brutishness = 25;
      leader.stats.skills.trickery = 10;
      giveStack(leader, "Lockpicks", 9, "picks");
      giveStack(leader, "Twine", 1, "not-key");
      auto prior = model::OpenedDoorRecord{};
      prior.layer = 1;
      prior.x = 0;
      prior.y = 0;
      prior.tileId = 99;
      mapOf(state).persistentState.openedDoors.pushBack(prior);
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "missing key stays.x") && ok;
      ok = assertEqual(avatar->y, 2, "missing key stays.y") && ok;
      ok = assertEqual(door->tileId, 2, "missing key tileId unchanged") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 9, "missing key does not spend tools") &&
           ok;
      ok = assertEqual(static_cast<int>(mapOf(state).persistentState.openedDoors.size()),
                       1,
                       "failed key bump leaves openedDoors unchanged") &&
           ok;
      ok = assertEqual(mapOf(state).persistentState.openedDoors[0].tileId,
                       99,
                       "failed key bump keeps prior record") &&
           ok;
      ok = assertFalse(soundsContain(state, "lockpick"), "missing key is silent") && ok;
    }

    // Lock level 0 does not bash at any Brutishness
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 0, "ShedKey");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.brutishness = 100;
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "level 0 bash stays.x") && ok;
      ok = assertEqual(door->tileId, 2, "level 0 bash tileId unchanged") && ok;
      ok = assertTrue(mapOf(state).persistentState.openedDoors.empty(),
                      "level 0 bash records nothing") &&
           ok;
      ok = assertFalse(soundsContain(state, "hit_punch1"), "level 0 bash plays nothing") && ok;
    }

    // Trickery that covers the cost opens and consumes no tools
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 10;
      giveStack(leader, "Lockpicks", 4, "picks");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "trickery cover bump.x") && ok;
      ok = assertEqual(door->tileId, 3, "trickery cover opened") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 4, "trickery cover consumes no tools") &&
           ok;
      ok = assertTrue(soundsContain(state, "lockpick"), "trickery cover plays lockpick") && ok;
      ok = assertFalse(soundsContain(state, "hit_punch1"), "trickery cover is not a bash") && ok;
    }

    // Partial tools do not open and do not change quantity
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 4;
      giveStack(leader, "Lockpicks", 5, "picks");
      auto prior = model::OpenedDoorRecord{};
      prior.layer = 0;
      prior.x = 1;
      prior.y = 1;
      prior.tileId = 53;
      mapOf(state).persistentState.openedDoors.pushBack(prior);
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "partial tools stay.x") && ok;
      ok = assertEqual(door->tileId, 2, "partial tools stay closed") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 5, "partial tools quantity unchanged") &&
           ok;
      ok = assertEqual(stackQuantityById(leader, "picks"), 5, "partial tools stack unchanged") &&
           ok;
      ok = assertEqual(static_cast<int>(mapOf(state).persistentState.openedDoors.size()),
                       1,
                       "partial tools leave openedDoors unchanged") &&
           ok;
      ok = assertEqual(mapOf(state).persistentState.openedDoors[0].tileId,
                       53,
                       "partial tools keep prior record") &&
           ok;
    }

    // Non-lock-tool stacks do not count toward the cost
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 4;
      giveStack(leader, "Twine", 6, "twine");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "non-tool stay.x") && ok;
      ok = assertEqual(door->tileId, 2, "non-tool stay closed") && ok;
      ok = assertEqual(itemQuantity(leader, "Twine"), 6, "non-tool quantity unchanged") && ok;
    }

    // Exact tools consume and open, leaving 0 of that stack
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 4;
      giveStack(leader, "Lockpicks", 6, "picks");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "exact tools bump.x") && ok;
      ok = assertEqual(door->tileId, 3, "exact tools opened") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 0, "exact tools leave 0") && ok;
      ok = assertEqual(stackQuantityById(leader, "picks"), -1, "exact tools stack removed") && ok;
      ok = assertTrue(soundsContain(state, "lockpick"), "exact tools play lockpick") && ok;
      ok = assertEqual(static_cast<int>(mapOf(state).persistentState.openedDoors.size()),
                       1,
                       "exact tools recorded") &&
           ok;
      if (!mapOf(state).persistentState.openedDoors.empty()) {
        ok = assertEqual(mapOf(state).persistentState.openedDoors[0].tileId,
                         3,
                         "exact tools record open tile") &&
             ok;
      }
    }

    // A larger stack leaves the exact remainder
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 4;
      giveStack(leader, "Lockpicks", 8, "picks");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(door->tileId, 3, "remainder tools opened") && ok;
      ok = assertEqual(avatar->x, 2, "remainder tools bump.x") && ok;
      ok = assertEqual(stackQuantityById(leader, "picks"), 2, "remainder tools leave 2") && ok;
    }

    // Split stacks consume the required total, or nothing when the sum is short
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 4;
      giveStack(leader, "Lockpicks", 2, "picks-a");
      giveStack(leader, "Lockpicks", 3, "picks-b");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(door->tileId, 2, "short split stays closed") && ok;
      ok = assertEqual(avatar->x, 2, "short split stays.x") && ok;
      ok = assertEqual(stackQuantityById(leader, "picks-a"), 2, "short split first unchanged") &&
           ok;
      ok = assertEqual(stackQuantityById(leader, "picks-b"), 3, "short split second unchanged") &&
           ok;
      ok = assertTrue(mapOf(state).persistentState.openedDoors.empty(),
                      "short split records nothing") &&
           ok;
    }

    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 4;
      giveStack(leader, "Lockpicks", 2, "picks-a");
      giveStack(leader, "Lockpicks", 4, "picks-b");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(door->tileId, 3, "split tools opened") && ok;
      ok = assertEqual(avatar->x, 2, "split tools bump.x") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 0, "split tools consume the total") &&
           ok;
      ok = assertEqual(stackQuantityById(leader, "picks-a"), -1, "split tools first stack gone") &&
           ok;
      ok = assertEqual(stackQuantityById(leader, "picks-b"), -1, "split tools second stack gone") &&
           ok;
    }

    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 4;
      giveStack(leader, "Twine", 9, "twine");
      giveStack(leader, "Lockpicks", 4, "picks-a");
      giveStack(leader, "Lockpicks", 5, "picks-b");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "split remainder bump.x") && ok;
      ok = assertEqual(door->tileId, 3, "split remainder opened") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 3, "split remainder total") && ok;
      ok = assertEqual(stackQuantityById(leader, "picks-a"), -1, "split remainder first consumed") &&
           ok;
      ok = assertEqual(stackQuantityById(leader, "picks-b"), 3, "split remainder second left") &&
           ok;
      ok = assertEqual(itemQuantity(leader, "Twine"), 9, "split remainder ignores non-tools") &&
           ok;
    }

    // Brutishness 5 bashes lock level 5; Brutishness 4 does not
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 5, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.brutishness = 4;
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "bash below threshold stays.x") && ok;
      ok = assertEqual(door->tileId, 2, "bash below threshold stays closed") && ok;
      ok = assertTrue(mapOf(state).persistentState.openedDoors.empty(),
                      "bash below threshold records nothing") &&
           ok;
      ok = assertFalse(soundsContain(state, "hit_punch1"), "bash below threshold is silent") &&
           ok;
    }

    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 5, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.brutishness = 5;
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "bash threshold bump.x") && ok;
      ok = assertEqual(door->tileId, 3, "bash threshold opened") && ok;
      ok = assertTrue(soundsContain(state, "hit_punch1"), "bash threshold plays punch") && ok;
      ok = assertFalse(soundsContain(state, "lockpick"), "bash threshold is not lockpick") && ok;
      ok = assertEqual(static_cast<int>(mapOf(state).persistentState.openedDoors.size()),
                       1,
                       "bash threshold recorded") &&
           ok;
      if (!mapOf(state).persistentState.openedDoors.empty()) {
        ok = assertEqual(mapOf(state).persistentState.openedDoors[0].tileId,
                         3,
                         "bash threshold record open tile") &&
             ok;
      }
    }

    // Bash wins over tools when Trickery does not cover the cost
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 5, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 0;
      leader.stats.skills.brutishness = 5;
      giveStack(leader, "Lockpicks", 5, "picks");
      auto* avatar =
          game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);

      move(state, 1, 0);
      ok = assertEqual(door->tileId, 3, "bash over tools opened") && ok;
      ok = assertEqual(avatar->x, 2, "bash over tools bump.x") && ok;
      ok = assertEqual(itemQuantity(leader, "Lockpicks"), 5, "bash over tools consumes nothing") &&
           ok;
      ok = assertTrue(soundsContain(state, "hit_punch1"), "bash over tools plays punch") && ok;
      ok = assertFalse(soundsContain(state, "lockpick"), "bash over tools is not lockpick") && ok;
    }

    // A rebuilt instance plus applyOpenedDoors restores the open tile id.
    // An empty openedDoors list leaves the template tile id unchanged.
    {
      auto& state = stateManager.getState();
      state = state::State{};
      setupGrid(database, state, 5, 5);
      state.world.activeMap.mapLayer = 0;
      auto* door = tileAt(mapOf(state), 3, 2, 0);
      door->tileId = 2;
      setLock(*door, 10, "");
      spawnAvatar(state, 2, 2);
      auto& leader = state.player.party[0];
      leader.stats.skills.trickery = 10;

      move(state, 1, 0);
      ok = assertEqual(door->tileId, 3, "persist open tileId") && ok;
      const auto& recorded = mapOf(state).persistentState.openedDoors;
      ok = assertEqual(static_cast<int>(recorded.size()), 1, "persist recorded one door") && ok;
      if (!recorded.empty()) {
        ok = assertEqual(recorded[0].tileId, 3, "persist record tileId") && ok;
        ok = assertEqual(recorded[0].x, 3, "persist record.x") && ok;
        ok = assertEqual(recorded[0].y, 2, "persist record.y") && ok;
        ok = assertEqual(recorded[0].layer, 0, "persist record.layer") && ok;
      }

      auto tmpl = makeClosedDoorTemplate(3, 2);
      auto rebuilt = model::createMapInstanceFromTemplate(tmpl);
      auto* rebuiltDoor = model::mapInstanceGetTileAt(rebuilt, 3, 2, 0);
      ok = assertTrue(rebuiltDoor != nullptr, "rebuilt door exists") && ok;
      if (rebuiltDoor) {
        ok = assertEqual(rebuiltDoor->tileId, 2, "rebuilt template starts closed") && ok;
      }
      rebuilt.persistentState.openedDoors = recorded;
      game::applyOpenedDoors(rebuilt, rebuilt.persistentState.openedDoors);
      rebuiltDoor = model::mapInstanceGetTileAt(rebuilt, 3, 2, 0);
      if (rebuiltDoor) {
        ok = assertEqual(rebuiltDoor->tileId, 3, "applyOpenedDoors restores open tile") && ok;
      }

      auto fresh = model::createMapInstanceFromTemplate(tmpl);
      game::applyOpenedDoors(fresh, fresh.persistentState.openedDoors);
      auto* freshDoor = model::mapInstanceGetTileAt(fresh, 3, 2, 0);
      ok = assertTrue(freshDoor != nullptr && freshDoor->tileId == 2,
                      "empty openedDoors leaves template tile") &&
           ok;

      database.addMapTemplate(tmpl);
      auto instances = game::createMapInstances(database);
      auto created = instances.find(bmin::String("rebuild_door_map"));
      ok = assertTrue(created != instances.end(), "createMapInstances has rebuild map") && ok;
      if (created != instances.end()) {
        auto* createdDoor = model::mapInstanceGetTileAt(created->value, 3, 2, 0);
        ok = assertTrue(createdDoor != nullptr && createdDoor->tileId == 2,
                        "fresh createMapInstances keeps closed tile id") &&
             ok;
      }
    }

    // Loaded tilesets from assets (terrain0 door pair sanity)
    {
      db::Database fullDb;
      fullDb.load();
      const auto& terrain0 = fullDb.getTilesetTemplate("terrain0");
      const auto* closed = game::findTileMetadata(terrain0, 52);
      const auto* open = game::findTileMetadata(terrain0, 53);
      ok = assertTrue(closed != nullptr, "terrain0 tile 52 exists") && ok;
      ok = assertTrue(open != nullptr, "terrain0 tile 53 exists") && ok;
      if (closed && open) {
        ok = assertTrue(closed->isDoor, "terrain0 52 isDoor") && ok;
        ok = assertFalse(closed->isWalkable, "terrain0 52 not walkable") && ok;
        ok = assertTrue(open->isDoor, "terrain0 53 isDoor") && ok;
        ok = assertTrue(open->isWalkable, "terrain0 53 walkable") && ok;
      }
    }

    if (!ok) {
      LOG(ERROR) << "TestWorldMovePlayer assertions failed" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestWorldMovePlayer completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
