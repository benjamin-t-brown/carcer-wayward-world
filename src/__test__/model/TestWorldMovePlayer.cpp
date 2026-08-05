#include "db/Database.h"
#include "game/map/Camera.h"
#include "game/map/MapWalkability.h"
#include "game/map/TileTriggers.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/MapGrids.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/WorldUpdater.h"
#include "state/actions/world/WorldMovePlayer.hpp"
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

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldMovePlayer" << LOG_ENDL;

  bool ok = true;

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    addTestTileset(database);

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
      move(state, 0, 1);
      ok = assertTrue(avatar->facing == model::CharacterFacing::Left, "walk down facing") && ok;
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

      state::worldUpdate(stateManager, 16);
      auto before = game::computeCameraFollow(
          2, 2, state.world.camera.viewW, state.world.camera.viewH);
      ok = assertEqual(state.world.camera.camX, before.camX, "cam before.x") && ok;
      ok = assertEqual(state.world.camera.camY, before.camY, "cam before.y") && ok;

      move(state, 1, 0);
      state::worldUpdate(stateManager, 16);
      auto after = game::computeCameraFollow(
          3, 2, state.world.camera.viewW, state.world.camera.viewH);
      ok = assertEqual(avatar->x, 3, "cam follow move.x") && ok;
      ok = assertEqual(state.world.camera.camX, after.camX, "cam after.x") && ok;
      ok = assertEqual(state.world.camera.camY, after.camY, "cam after.y") && ok;
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
