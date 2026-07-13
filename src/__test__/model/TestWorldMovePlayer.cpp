#include "db/Database.h"
#include "model/MapWalkability.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/World.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
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
  map.width = width;
  map.height = height;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  map.templateName = "test_map";
  map.tileLayerNumber = 0;
  mapLayerAt(map.tiles, 0) = makeLayerTiles(width, height, 0);
  return map;
}

model::TileInstance* tileAt(model::MapInstance& map, int x, int y, int layer = 0) {
  auto index = y * map.width + x;
  return &map.tiles[layer][static_cast<size_t>(index)];
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
  state.world.currentMap.characters.pushBack(std::move(avatar));
  return &state.world.currentMap.characters[0];
}

void move(state::State& state, int dx, int dy) {
  auto action = state::actions::WorldMovePlayer(dx, dy);
  action.execute(&state);
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestWorldMovePlayer" << LOG_ENDL;

  bool ok = true;

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    addTestTileset(database);

    // Pure helper: override true vs non-walkable tileset
    {
      auto tile = makeTile(0, 0, 1);
      ok = assertFalse(model::isTileEffectivelyWalkable(tile, database),
                       "wall without override walkable") &&
           ok;
      tile.tileOverrides = model::TileOverrides{};
      tile.tileOverrides->isWalkableOverride = true;
      ok = assertTrue(model::isTileEffectivelyWalkable(tile, database),
                      "override true wins over wall") &&
           ok;
    }

    // Pure helper: override false vs walkable tileset
    {
      auto tile = makeTile(0, 0, 0);
      ok = assertTrue(model::isTileEffectivelyWalkable(tile, database),
                      "floor without override walkable") &&
           ok;
      tile.tileOverrides = model::TileOverrides{};
      tile.tileOverrides->isWalkableOverride = false;
      ok = assertFalse(model::isTileEffectivelyWalkable(tile, database),
                       "override false wins over floor") &&
           ok;
    }

    // Walk onto walkable tile
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "walk right.x") && ok;
      ok = assertEqual(avatar->y, 2, "walk right.y") && ok;
    }

    // Blocked by non-walkable non-door
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      tileAt(state.world.currentMap, 3, 2)->tileId = 1;
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "blocked wall.x") && ok;
      ok = assertEqual(avatar->y, 2, "blocked wall.y") && ok;
    }

    // Override true allows move onto tileset-non-walkable
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      auto* dest = tileAt(state.world.currentMap, 3, 2);
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
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      auto* dest = tileAt(state.world.currentMap, 3, 2);
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
      ok = assertTrue(model::isTileEffectivelyWalkable(tile, database),
                      "empty overrides object still walkable") &&
           ok;
    }

    // Current layer only: wall on another layer must not block walkable current layer
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      state.world.currentMap.tileLayerNumber = 0;
      model::mapLayerAt(state.world.currentMap.tiles, 1) = makeLayerTiles(5, 5, 1); // wall on layer 1
      // layer 0 dest stays floor (tileId 0)
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "other-layer wall ignored.x") && ok;
      ok = assertEqual(avatar->y, 2, "other-layer wall ignored.y") && ok;
    }

    // Current layer only: wall on tileLayerNumber blocks even if other layers are floor
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      model::mapLayerAt(state.world.currentMap.tiles, 1) = makeLayerTiles(5, 5, 0); // floor on layer 1
      tileAt(state.world.currentMap, 3, 2, 0)->tileId = 1; // wall on layer 0
      state.world.currentMap.tileLayerNumber = 0;
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "current-layer wall blocks.x") && ok;
      ok = assertEqual(avatar->y, 2, "current-layer wall blocks.y") && ok;
    }

    // Current layer only: pointing tileLayerNumber at a wall layer blocks
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      model::mapLayerAt(state.world.currentMap.tiles, 1) = makeLayerTiles(5, 5, 1); // wall on layer 1
      state.world.currentMap.tileLayerNumber = 1;
      auto* avatar = spawnAvatar(state, 2, 2);
      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "tileLayerNumber wall blocks.x") && ok;
      ok = assertEqual(avatar->y, 2, "tileLayerNumber wall blocks.y") && ok;
    }

    // Closed door: open without moving, then walk through
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      state.world.currentMap.tileLayerNumber = 0;
      auto* door = tileAt(state.world.currentMap, 3, 2, 0);
      door->tileId = 2; // closed door
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

    // Already-open door (isDoor + walkable meta): move onto it, tileId unchanged
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      state.world.currentMap.tileLayerNumber = 0;
      auto* door = tileAt(state.world.currentMap, 3, 2, 0);
      door->tileId = 3; // open door
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 3, "already-open door move.x") && ok;
      ok = assertEqual(avatar->y, 2, "already-open door move.y") && ok;
      ok = assertEqual(door->tileId, 3, "already-open door tileId unchanged") && ok;
    }

    // Closed door with walkable override still opens (tileset classification)
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      state.world.currentMap.tileLayerNumber = 0;
      auto* door = tileAt(state.world.currentMap, 3, 2, 0);
      door->tileId = 2; // closed door meta
      door->tileOverrides = model::TileOverrides{};
      door->tileOverrides->isWalkableOverride = true;
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "closed+override still opens.x") && ok;
      ok = assertEqual(avatar->y, 2, "closed+override still opens.y") && ok;
      ok = assertEqual(door->tileId, 3, "closed+override opened tileId") && ok;
    }

    // Open door with non-walkable override: not re-opened; move blocked by override
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      state.world.currentMap.tileLayerNumber = 0;
      auto* door = tileAt(state.world.currentMap, 3, 2, 0);
      door->tileId = 3; // open door meta
      door->tileOverrides = model::TileOverrides{};
      door->tileOverrides->isWalkableOverride = false;
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "open+override false no move.x") && ok;
      ok = assertEqual(avatar->y, 2, "open+override false no move.y") && ok;
      ok = assertEqual(door->tileId, 3, "open+override false tileId unchanged") && ok;
    }

    // Door open mutates only the current layer; other-layer door untouched
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      model::mapLayerAt(state.world.currentMap.tiles, 1) = makeLayerTiles(5, 5, 0);
      auto* doorLayer0 = tileAt(state.world.currentMap, 3, 2, 0);
      auto* doorLayer1 = tileAt(state.world.currentMap, 3, 2, 1);
      doorLayer0->tileId = 2; // closed door on current layer
      doorLayer1->tileId = 2; // closed door on other layer
      state.world.currentMap.tileLayerNumber = 0;
      auto* avatar = spawnAvatar(state, 2, 2);

      move(state, 1, 0);
      ok = assertEqual(avatar->x, 2, "current-layer door bump.x") && ok;
      ok = assertEqual(doorLayer0->tileId, 3, "current-layer door opened") && ok;
      ok = assertEqual(doorLayer1->tileId, 2, "other-layer door unchanged") && ok;
    }

    // Out of bounds no-op
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(5, 5);
      auto* avatar = spawnAvatar(state, 0, 0);
      move(state, -1, 0);
      ok = assertEqual(avatar->x, 0, "oob left.x") && ok;
      ok = assertEqual(avatar->y, 0, "oob left.y") && ok;
      move(state, 0, -1);
      ok = assertEqual(avatar->x, 0, "oob up.x") && ok;
      ok = assertEqual(avatar->y, 0, "oob up.y") && ok;
    }

    // Camera follow recenters after successful move
    {
      state::State state;
      state.world.currentMap = makeEmptyMap(10, 10);
      state.world.viewW = 100;
      state.world.viewH = 80;
      state.world.cameraMode = model::CameraMode::Follow;
      auto* avatar = spawnAvatar(state, 2, 2);
      state.world.cameraFollowCharacterId = avatar->id;

      state::worldUpdate(state, 16);
      auto before = model::computeCameraFollow(
          2, 2, state.world.currentMap, state.world.viewW, state.world.viewH);
      ok = assertEqual(state.world.camX, before.camX, "cam before.x") && ok;
      ok = assertEqual(state.world.camY, before.camY, "cam before.y") && ok;

      move(state, 1, 0);
      state::worldUpdate(state, 16);
      auto after = model::computeCameraFollow(
          3, 2, state.world.currentMap, state.world.viewW, state.world.viewH);
      ok = assertEqual(avatar->x, 3, "cam follow move.x") && ok;
      ok = assertEqual(state.world.camX, after.camX, "cam after.x") && ok;
      ok = assertEqual(state.world.camY, after.camY, "cam after.y") && ok;
    }

    // Loaded tilesets from assets (terrain0 door pair sanity)
    {
      db::Database fullDb;
      fullDb.load();
      const auto& terrain0 = fullDb.getTilesetTemplate("terrain0");
      const auto* closed = model::findTileMetadata(terrain0, 52);
      const auto* open = model::findTileMetadata(terrain0, 53);
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
