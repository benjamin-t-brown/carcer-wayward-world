#include "db/Database.h"
#include "model/MapPersistence.h"
#include "model/MapVision.h"
#include "model/MapWalkability.h"
#include "model/instances/World.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "bmin/String.h"

namespace {

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

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

model::TileMetadata makeMeta(int id, bool walkable, bool isSeeThrough, bool isDoor = false) {
  auto meta = model::TileMetadata{};
  meta.id = id;
  meta.isWalkable = walkable;
  meta.isDoor = isDoor;
  meta.isSeeThrough = isSeeThrough;
  return meta;
}

void addTestTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  tileset.tileWidth = 28;
  tileset.tileHeight = 32;
  // 0 floor (see-through), 1 wall (opaque), 2 closed door, 3 open door
  tileset.tiles.pushBack(makeMeta(0, true, true));
  tileset.tiles.pushBack(makeMeta(1, false, false));
  tileset.tiles.pushBack(makeMeta(2, false, false, true));
  tileset.tiles.pushBack(makeMeta(3, true, true, true));
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
  model::mapLayerAt(map.tiles, 0) = makeLayerTiles(width, height, 0);
  return map;
}

model::TileInstance* tileAt(model::MapInstance& map, int x, int y, int layer = 0) {
  auto index = y * map.width + x;
  return &map.tiles[layer][static_cast<size_t>(index)];
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestMapVision" << LOG_ENDL;

  bool ok = true;

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    addTestTileset(database);

    // Pure helper: override true vs non-see-through tileset (parallel to walkable)
    {
      auto tile = makeTile(0, 0, 1);
      ok = assertFalse(model::isTileEffectivelySeeThrough(tile, database),
                       "wall without override see-through") &&
           ok;
      ok = assertTrue(model::doesTileBlockSight(tile, database),
                      "wall without override blocks sight") &&
           ok;
      tile.tileOverrides = model::TileOverrides{};
      tile.tileOverrides->isSeeThroughOverride = true;
      ok = assertTrue(model::isTileEffectivelySeeThrough(tile, database),
                      "override true wins over wall") &&
           ok;
      ok = assertFalse(model::doesTileBlockSight(tile, database),
                       "override true does not block sight") &&
           ok;
    }

    // Pure helper: override false vs see-through tileset
    {
      auto tile = makeTile(0, 0, 0);
      ok = assertTrue(model::isTileEffectivelySeeThrough(tile, database),
                      "floor without override see-through") &&
           ok;
      tile.tileOverrides = model::TileOverrides{};
      tile.tileOverrides->isSeeThroughOverride = false;
      ok = assertFalse(model::isTileEffectivelySeeThrough(tile, database),
                       "override false wins over floor") &&
           ok;
    }

    // Empty TileOverrides object (no authored isSeeThroughOverride) uses metadata
    {
      auto tile = makeTile(0, 0, 0);
      tile.tileOverrides = model::TileOverrides{};
      ok = assertTrue(model::isTileEffectivelySeeThrough(tile, database),
                      "empty overrides object still see-through") &&
           ok;
    }

    // Empty tilesetName → see-through
    {
      auto tile = model::TileInstance{};
      tile.x = 0;
      tile.y = 0;
      ok = assertTrue(model::isTileEffectivelySeeThrough(tile, database),
                      "empty tilesetName see-through") &&
           ok;
    }

    // Fresh map / before update: tiles not explored and not visible
    {
      auto map = makeEmptyMap(5, 5);
      const auto* t = tileAt(map, 2, 2);
      ok = assertFalse(t->isExplored, "fresh tile not explored") && ok;
      ok = assertFalse(t->isVisible, "fresh tile not visible") && ok;
      ok = assertFalse(tileAt(map, 0, 0)->isExplored, "fresh corner not explored") && ok;
      ok = assertFalse(tileAt(map, 0, 0)->isVisible, "fresh corner not visible") && ok;
    }

    // Player cell always visible after update
    {
      auto map = makeEmptyMap(5, 5);
      model::updateMapVisibilityFromPlayer(map, 2, 2, database);
      const auto* playerTile = tileAt(map, 2, 2);
      ok = assertTrue(playerTile->isVisible, "player cell visible") && ok;
      ok = assertTrue(playerTile->isExplored, "player cell explored") && ok;
    }

    // Wall blocks further tiles along a ray; wall tile itself can be visible
    {
      auto map = makeEmptyMap(7, 7);
      // Horizontal ray east from (1,3): wall at (3,3), floor beyond at (4,3) and (5,3)
      tileAt(map, 3, 3)->tileId = 1;
      model::updateMapVisibilityFromPlayer(map, 1, 3, database);

      ok = assertTrue(tileAt(map, 1, 3)->isVisible, "player visible with wall ahead") && ok;
      ok = assertTrue(tileAt(map, 2, 3)->isVisible, "tile before wall visible") && ok;
      ok = assertTrue(tileAt(map, 3, 3)->isVisible, "wall tile itself visible") && ok;
      ok = assertTrue(tileAt(map, 3, 3)->isExplored, "wall tile explored") && ok;
      ok = assertFalse(tileAt(map, 4, 3)->isVisible, "tile beyond wall not visible") && ok;
      ok = assertFalse(tileAt(map, 5, 3)->isVisible, "far tile beyond wall not visible") && ok;
      ok = assertFalse(tileAt(map, 4, 3)->isExplored, "tile beyond wall not explored") && ok;
    }

    // Continuous wall face beside the player is fully lit within the vision box
    // (perimeter Bresenham alone leaves gaps along opaque columns).
    {
      auto map = makeEmptyMap(20, 20);
      const auto playerX = 5;
      const auto playerY = 10;
      for (auto y = 0; y < map.height; y++) {
        tileAt(map, playerX + 1, y)->tileId = 1; // opaque wall column
      }
      model::updateMapVisibilityFromPlayer(map, playerX, playerY, database);

      for (auto y = playerY - model::kPlayerVisionBoxSize;
           y <= playerY + model::kPlayerVisionBoxSize; y++) {
        if (!model::isInPlayerVisionRange(1, y - playerY)) {
          continue;
        }
        ok = assertTrue(tileAt(map, playerX + 1, y)->isVisible,
                        "wall face tile visible along column") &&
             ok;
      }
      ok = assertFalse(tileAt(map, playerX + 2, playerY)->isVisible,
                       "tile beyond wall column not visible") &&
           ok;
    }

    // Vision shape is an octagon: axis extremes lit, square corners dark
    {
      auto map = makeEmptyMap(20, 20);
      const auto px = 10;
      const auto py = 10;
      const auto r = model::kPlayerVisionBoxSize;
      model::updateMapVisibilityFromPlayer(map, px, py, database);

      ok = assertTrue(tileAt(map, px + r, py)->isVisible, "east extent visible") && ok;
      ok = assertTrue(tileAt(map, px, py + r)->isVisible, "south extent visible") && ok;
      ok = assertFalse(tileAt(map, px + r, py + r)->isVisible,
                       "square corner outside octagon") &&
           ok;
      ok = assertFalse(model::isInPlayerVisionRange(r, r, r), "corner not in range helper") &&
           ok;
      ok = assertTrue(model::isInPlayerVisionRange(r, 0, r), "axis in range helper") && ok;
    }

    // Override false on floor blocks sight along ray (like opaque wall)
    {
      auto map = makeEmptyMap(7, 7);
      auto* blocker = tileAt(map, 3, 3);
      blocker->tileId = 0;
      blocker->tileOverrides = model::TileOverrides{};
      blocker->tileOverrides->isSeeThroughOverride = false;
      model::updateMapVisibilityFromPlayer(map, 1, 3, database);

      ok = assertTrue(tileAt(map, 3, 3)->isVisible, "opaque-override tile visible") && ok;
      ok = assertFalse(tileAt(map, 4, 3)->isVisible, "beyond opaque-override not visible") &&
           ok;
    }

    // Override true on wall allows seeing past it
    {
      auto map = makeEmptyMap(7, 7);
      auto* wall = tileAt(map, 3, 3);
      wall->tileId = 1;
      wall->tileOverrides = model::TileOverrides{};
      wall->tileOverrides->isSeeThroughOverride = true;
      model::updateMapVisibilityFromPlayer(map, 1, 3, database);

      ok = assertTrue(tileAt(map, 3, 3)->isVisible, "see-through wall visible") && ok;
      ok = assertTrue(tileAt(map, 4, 3)->isVisible, "beyond see-through wall visible") && ok;
    }

    // After move away: previously visible tiles remain explored, become not visible
    {
      auto map = makeEmptyMap(20, 20);
      model::updateMapVisibilityFromPlayer(map, 2, 2, database);

      ok = assertTrue(tileAt(map, 2, 2)->isVisible, "first pos player visible") && ok;
      ok = assertTrue(tileAt(map, 3, 2)->isVisible, "neighbor visible before move") && ok;
      ok = assertTrue(tileAt(map, 3, 2)->isExplored, "neighbor explored before move") && ok;

      // Move far outside vision box (kPlayerVisionBoxSize = 7)
      model::updateMapVisibilityFromPlayer(map, 15, 15, database);

      ok = assertTrue(tileAt(map, 15, 15)->isVisible, "new player cell visible") && ok;
      ok = assertTrue(tileAt(map, 15, 15)->isExplored, "new player cell explored") && ok;
      ok = assertFalse(tileAt(map, 2, 2)->isVisible, "old player cell no longer visible") && ok;
      ok = assertTrue(tileAt(map, 2, 2)->isExplored, "old player cell still explored") && ok;
      ok = assertFalse(tileAt(map, 3, 2)->isVisible, "old neighbor no longer visible") && ok;
      ok = assertTrue(tileAt(map, 3, 2)->isExplored, "old neighbor still explored") && ok;
      // Outside both vision boxes (player at 2,2 then 15,15; boxSize = 7)
      ok = assertFalse(tileAt(map, 19, 0)->isExplored, "never-seen tile still not explored") &&
           ok;
      ok = assertFalse(tileAt(map, 19, 0)->isVisible, "never-seen tile still not visible") && ok;
    }

    // Flags sync across all layers at a visible cell
    {
      auto map = makeEmptyMap(5, 5);
      model::mapLayerAt(map.tiles, 1) = makeLayerTiles(5, 5, 0);
      model::updateMapVisibilityFromPlayer(map, 2, 2, database);
      ok = assertTrue(tileAt(map, 2, 2, 0)->isVisible, "layer0 player visible") && ok;
      ok = assertTrue(tileAt(map, 2, 2, 1)->isVisible, "layer1 player visible") && ok;
      ok = assertTrue(tileAt(map, 2, 2, 1)->isExplored, "layer1 player explored") && ok;
    }

    // Explored mask survives map leave/return (MapPersistence flush/hydrate)
    {
      auto map = makeEmptyMap(20, 20);
      model::updateMapVisibilityFromPlayer(map, 2, 2, database);
      ok = assertTrue(tileAt(map, 2, 2)->isExplored, "explored before capture") && ok;
      ok = assertTrue(tileAt(map, 3, 2)->isExplored, "neighbor explored before capture") && ok;

      model::World world;
      world.currentMap = map;
      model::flushCurrentMapToPersistence(world, database);

      // Simulate leaving: wipe tiles, then hydrate from world.mapsByTemplate
      world.currentMap = makeEmptyMap(20, 20);
      ok = assertFalse(tileAt(world.currentMap, 2, 2)->isExplored,
                       "fresh map not explored before restore") &&
           ok;
      model::hydrateCurrentMapFromPersistence(world, database);

      ok = assertTrue(tileAt(world.currentMap, 2, 2)->isExplored,
                      "player cell explored after restore") &&
           ok;
      ok = assertTrue(tileAt(world.currentMap, 3, 2)->isExplored,
                      "neighbor explored after restore") &&
           ok;
      ok = assertFalse(tileAt(world.currentMap, 2, 2)->isVisible,
                       "visibility not restored (recomputed on spawn)") &&
           ok;
      ok = assertFalse(tileAt(world.currentMap, 19, 0)->isExplored,
                       "never-seen still not explored after restore") &&
           ok;
    }

    // Open doors survive map leave/return
    {
      auto map = makeEmptyMap(8, 8);
      auto* door = tileAt(map, 4, 4);
      door->tileId = 3; // open door
      ok = assertTrue(model::isOpenDoorTile(*door, database), "fixture is open door") && ok;

      model::World world;
      world.currentMap = map;
      model::flushCurrentMapToPersistence(world, database);

      world.currentMap = makeEmptyMap(8, 8);
      auto* resetDoor = tileAt(world.currentMap, 4, 4);
      resetDoor->tileId = 2; // closed again (as if from template)
      ok = assertTrue(model::isClosedDoorTile(*resetDoor, database),
                      "door closed before restore") &&
           ok;

      model::hydrateCurrentMapFromPersistence(world, database);
      ok = assertEqual(tileAt(world.currentMap, 4, 4)->tileId, 3,
                       "open door tileId restored") &&
           ok;
      ok = assertTrue(model::isOpenDoorTile(*tileAt(world.currentMap, 4, 4), database),
                      "door open after restore") &&
           ok;
    }

    if (!ok) {
      LOG(ERROR) << "TestMapVision assertions failed" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestMapVision completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
