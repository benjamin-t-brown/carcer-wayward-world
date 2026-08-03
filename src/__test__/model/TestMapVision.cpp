#include "db/Database.h"
#include "game/map/MapVision.h"
#include "game/map/MapWalkability.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/instances/MapInstance.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/MapGrids.h"
#include "model/templates/Tileset.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
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
  tileset.tiles.pushBack(makeMeta(0, true, true));
  tileset.tiles.pushBack(makeMeta(1, false, false));
  tileset.tiles.pushBack(makeMeta(2, false, false, true));
  tileset.tiles.pushBack(makeMeta(3, true, true, true));
  database.addTilesetTemplate(tileset);
}

void addAllyCharacterTemplate(db::Database& database) {
  auto character = model::CharacterTemplate{};
  character.type = model::CharacterTemplateType::TOWNSPERSON;
  character.name = "ally";
  database.addCharacterTemplate(character);
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

model::MapInstance makeEmptyMap(const char* name, int width, int height) {
  auto map = model::MapInstance{};
  map.id = name;
  map.templateName = name;
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

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestMapVision" << LOG_ENDL;

  bool ok = true;

  try {
    db::Database database;
    state::DatabaseInterface::setDatabase(&database);
    addTestTileset(database);

    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);

    // Pure helper: override true vs non-see-through tileset
    {
      auto tile = makeTile(0, 0, 1);
      ok = assertFalse(game::isTileEffectivelySeeThrough(tile, database),
                       "wall without override see-through") &&
           ok;
      ok = assertTrue(game::doesTileBlockSight(tile, database),
                      "wall without override blocks sight") &&
           ok;
      tile.tileOverrides = model::TileOverrides{};
      tile.tileOverrides->isSeeThroughOverride = true;
      ok = assertTrue(game::isTileEffectivelySeeThrough(tile, database),
                      "override true wins over wall") &&
           ok;
      ok = assertFalse(game::doesTileBlockSight(tile, database),
                       "override true does not block sight") &&
           ok;
    }

    // Pure helper: override false vs see-through tileset
    {
      auto tile = makeTile(0, 0, 0);
      ok = assertTrue(game::isTileEffectivelySeeThrough(tile, database),
                      "floor without override see-through") &&
           ok;
      tile.tileOverrides = model::TileOverrides{};
      tile.tileOverrides->isSeeThroughOverride = false;
      ok = assertFalse(game::isTileEffectivelySeeThrough(tile, database),
                       "override false wins over floor") &&
           ok;
    }

    // Empty TileOverrides object uses metadata
    {
      auto tile = makeTile(0, 0, 0);
      tile.tileOverrides = model::TileOverrides{};
      ok = assertTrue(game::isTileEffectivelySeeThrough(tile, database),
                      "empty overrides object still see-through") &&
           ok;
    }

    // Empty tilesetName → see-through
    {
      auto tile = model::TileInstance{};
      tile.x = 0;
      tile.y = 0;
      ok = assertTrue(game::isTileEffectivelySeeThrough(tile, database),
                      "empty tilesetName see-through") &&
           ok;
    }

    // Fresh map / before update: tiles not explored and not visible
    {
      auto map = makeEmptyMap("test_map", 5, 5);
      const auto* t = tileAt(map, 2, 2);
      ok = assertFalse(t->isExplored, "fresh tile not explored") && ok;
      ok = assertFalse(t->isVisible, "fresh tile not visible") && ok;
    }

    // Player cell always visible after update
    {
      auto map = makeEmptyMap("test_map", 5, 5);
      game::updateMapVisibilityFromPlayer(map, 2, 2, database);
      const auto* playerTile = tileAt(map, 2, 2);
      ok = assertTrue(playerTile->isVisible, "player cell visible") && ok;
      ok = assertTrue(playerTile->isExplored, "player cell explored") && ok;
    }

    // Wall blocks further tiles along a ray
    {
      auto map = makeEmptyMap("test_map", 7, 7);
      tileAt(map, 3, 3)->tileId = 1;
      game::updateMapVisibilityFromPlayer(map, 1, 3, database);

      ok = assertTrue(tileAt(map, 1, 3)->isVisible, "player visible with wall ahead") && ok;
      ok = assertTrue(tileAt(map, 2, 3)->isVisible, "tile before wall visible") && ok;
      ok = assertTrue(tileAt(map, 3, 3)->isVisible, "wall tile itself visible") && ok;
      ok = assertFalse(tileAt(map, 4, 3)->isVisible, "tile beyond wall not visible") && ok;
      ok = assertFalse(tileAt(map, 4, 3)->isExplored, "tile beyond wall not explored") && ok;
    }

    // Continuous wall face beside the player is fully lit
    {
      auto map = makeEmptyMap("test_map", 20, 20);
      const auto playerX = 5;
      const auto playerY = 10;
      for (auto y = 0; y < map.height; y++) {
        tileAt(map, playerX + 1, y)->tileId = 1;
      }
      game::updateMapVisibilityFromPlayer(map, playerX, playerY, database);

      for (auto y = playerY - game::kPlayerVisionBoxSize;
           y <= playerY + game::kPlayerVisionBoxSize; y++) {
        if (!game::isInPlayerVisionRange(1, y - playerY)) {
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

    // Vision shape is an octagon
    {
      auto map = makeEmptyMap("test_map", 20, 20);
      const auto px = 10;
      const auto py = 10;
      const auto r = game::kPlayerVisionBoxSize;
      game::updateMapVisibilityFromPlayer(map, px, py, database);

      ok = assertTrue(tileAt(map, px + r, py)->isVisible, "east extent visible") && ok;
      ok = assertTrue(tileAt(map, px, py + r)->isVisible, "south extent visible") && ok;
      ok = assertFalse(tileAt(map, px + r, py + r)->isVisible,
                       "square corner outside octagon") &&
           ok;
      ok = assertFalse(game::isInPlayerVisionRange(r, r, r), "corner not in range helper") &&
           ok;
      ok = assertTrue(game::isInPlayerVisionRange(r, 0, r), "axis in range helper") && ok;
    }

    // Override false on floor blocks sight along ray
    {
      auto map = makeEmptyMap("test_map", 7, 7);
      auto* blocker = tileAt(map, 3, 3);
      blocker->tileId = 0;
      blocker->tileOverrides = model::TileOverrides{};
      blocker->tileOverrides->isSeeThroughOverride = false;
      game::updateMapVisibilityFromPlayer(map, 1, 3, database);

      ok = assertTrue(tileAt(map, 3, 3)->isVisible, "opaque-override tile visible") && ok;
      ok = assertFalse(tileAt(map, 4, 3)->isVisible, "beyond opaque-override not visible") &&
           ok;
    }

    // Override true on wall allows seeing past it
    {
      auto map = makeEmptyMap("test_map", 7, 7);
      auto* wall = tileAt(map, 3, 3);
      wall->tileId = 1;
      wall->tileOverrides = model::TileOverrides{};
      wall->tileOverrides->isSeeThroughOverride = true;
      game::updateMapVisibilityFromPlayer(map, 1, 3, database);

      ok = assertTrue(tileAt(map, 3, 3)->isVisible, "see-through wall visible") && ok;
      ok = assertTrue(tileAt(map, 4, 3)->isVisible, "beyond see-through wall visible") && ok;
    }

    // After move away: previously visible tiles remain explored
    {
      auto map = makeEmptyMap("test_map", 20, 20);
      game::updateMapVisibilityFromPlayer(map, 2, 2, database);

      ok = assertTrue(tileAt(map, 2, 2)->isVisible, "first pos player visible") && ok;
      ok = assertTrue(tileAt(map, 3, 2)->isExplored, "neighbor explored before move") && ok;

      game::updateMapVisibilityFromPlayer(map, 15, 15, database);

      ok = assertTrue(tileAt(map, 15, 15)->isVisible, "new player cell visible") && ok;
      ok = assertFalse(tileAt(map, 2, 2)->isVisible, "old player cell no longer visible") && ok;
      ok = assertTrue(tileAt(map, 2, 2)->isExplored, "old player cell still explored") && ok;
      ok = assertTrue(tileAt(map, 3, 2)->isExplored, "old neighbor still explored") && ok;
      ok = assertFalse(tileAt(map, 19, 0)->isExplored, "never-seen tile still not explored") &&
           ok;
    }

    // Combined party vision via active map (world coords)
    {
      addAllyCharacterTemplate(database);
      auto& state = stateManager.getState();
      state = state::State{};

      auto map = makeEmptyMap("vision_map", 20, 20);
      state.mapInstances[map.templateName] = std::move(map);

      model::MapGridTemplate grid;
      grid.name = "vision_grid";
      grid.gridWidth = 1;
      grid.gridHeight = 1;
      grid.mapWidth = 20;
      grid.mapHeight = 20;
      grid.cells = {{"vision_map"}};
      database.addMapGridTemplate(grid);
      state.world.activeMap.gridId = "vision_grid";

      auto memberNear = model::CharacterPlayer(database.getCharacterTemplate("ally"));
      memberNear.instanceId = "party-near";
      state.player.party.pushBack(std::move(memberNear));
      auto memberFar = model::CharacterPlayer(database.getCharacterTemplate("ally"));
      memberFar.instanceId = "party-far";
      state.player.party.pushBack(std::move(memberFar));

      state.world.activeMap.characters.pushBack(model::CharacterInstance{
          .id = "party-near",
          .templateName = "ally",
          .x = 2,
          .y = 2,
      });
      state.world.activeMap.characters.pushBack(model::CharacterInstance{
          .id = "party-far",
          .templateName = "ally",
          .x = 15,
          .y = 2,
      });
      state.world.activeMap.characters.pushBack(model::CharacterInstance{
          .id = "npc-ally",
          .templateName = "ally",
          .x = 15,
          .y = 2,
      });

      game::updateActiveMapVisibilityFromPlayer(state.world, 2, 2, database);
      ok = assertFalse(tileAt(state.mapInstances["vision_map"], 15, 2)->isVisible,
                       "distant party member not visible from single observer") &&
           ok;

      game::updateActiveMapVisibilityFromParty(state.world, state.player, database);
      ok = assertTrue(tileAt(state.mapInstances["vision_map"], 15, 2)->isVisible,
                      "distant party member visible with combined party vision") &&
           ok;
      ok = assertTrue(tileAt(state.mapInstances["vision_map"], 2, 2)->isVisible,
                      "near party member still visible") &&
           ok;

      state.player.party.clear();
      game::updateActiveMapVisibilityFromParty(state.world, state.player, database);
      ok = assertFalse(tileAt(state.mapInstances["vision_map"], 15, 2)->isVisible,
                       "npc ally does not contribute to party vision") &&
           ok;
    }

    // Flags sync across all layers at a visible cell
    {
      auto map = makeEmptyMap("test_map", 5, 5);
      model::mapLayerAt(model::mapInstanceTiles(map), 1) = makeLayerTiles(5, 5, 0);
      game::updateMapVisibilityFromPlayer(map, 2, 2, database);
      ok = assertTrue(tileAt(map, 2, 2, 0)->isVisible, "layer0 player visible") && ok;
      ok = assertTrue(tileAt(map, 2, 2, 1)->isVisible, "layer1 player visible") && ok;
      ok = assertTrue(tileAt(map, 2, 2, 1)->isExplored, "layer1 player explored") && ok;
    }

    // Explored mask capture/apply (MapInstance-local persistence)
    {
      auto map = makeEmptyMap("test_map", 20, 20);
      game::updateMapVisibilityFromPlayer(map, 2, 2, database);
      ok = assertTrue(tileAt(map, 2, 2)->isExplored, "explored before capture") && ok;
      ok = assertTrue(tileAt(map, 3, 2)->isExplored, "neighbor explored before capture") && ok;

      const auto mask = game::captureExploredMask(map);
      auto restored = makeEmptyMap("test_map", 20, 20);
      ok = assertFalse(tileAt(restored, 2, 2)->isExplored,
                       "fresh map not explored before restore") &&
           ok;
      game::applyExploredMask(restored, mask);

      ok = assertTrue(tileAt(restored, 2, 2)->isExplored,
                      "player cell explored after restore") &&
           ok;
      ok = assertTrue(tileAt(restored, 3, 2)->isExplored,
                      "neighbor explored after restore") &&
           ok;
      ok = assertFalse(tileAt(restored, 2, 2)->isVisible,
                       "visibility not restored (recomputed on spawn)") &&
           ok;
      ok = assertFalse(tileAt(restored, 19, 0)->isExplored,
                       "never-seen still not explored after restore") &&
           ok;
    }

    // Open doors survive via capture/apply on MapInstance
    {
      auto map = makeEmptyMap("test_map", 8, 8);
      auto* door = tileAt(map, 4, 4);
      door->tileId = 3;
      ok = assertTrue(game::isOpenDoorTile(*door, database), "fixture is open door") && ok;

      const auto doors = game::captureOpenedDoors(map, database);
      auto restored = makeEmptyMap("test_map", 8, 8);
      auto* resetDoor = tileAt(restored, 4, 4);
      resetDoor->tileId = 2;
      ok = assertTrue(game::isClosedDoorTile(*resetDoor, database),
                      "door closed before restore") &&
           ok;

      game::applyOpenedDoors(restored, doors);
      ok = assertEqual(tileAt(restored, 4, 4)->tileId, 3, "open door tileId restored") &&
           ok;
      ok = assertTrue(game::isOpenDoorTile(*tileAt(restored, 4, 4), database),
                      "door open after restore") &&
           ok;
    }

    // Cross-stitch vision: player near map edge lights adjacent map instance
    {
      auto& state = stateManager.getState();
      state = state::State{};

      constexpr int mapW = 8;
      constexpr int mapH = 8;
      state.mapInstances["west_map"] = makeEmptyMap("west_map", mapW, mapH);
      state.mapInstances["east_map"] = makeEmptyMap("east_map", mapW, mapH);

      model::MapGridTemplate grid;
      grid.name = "stitch_grid";
      grid.gridWidth = 2;
      grid.gridHeight = 1;
      grid.mapWidth = mapW;
      grid.mapHeight = mapH;
      grid.cells = {{"west_map", "east_map"}};
      database.addMapGridTemplate(grid);
      state.world.activeMap.gridId = "stitch_grid";
      state.world.activeMap.mapLayer = 0;

      // Player on west map at the stitch edge (local 7,4 → world 7,4)
      const auto playerWorldX = mapW - 1;
      const auto playerWorldY = 4;
      game::updateActiveMapVisibilityFromPlayer(
          state.world, playerWorldX, playerWorldY, database);

      auto& west = state.mapInstances["west_map"];
      auto& east = state.mapInstances["east_map"];
      ok = assertTrue(tileAt(west, mapW - 1, 4)->isVisible,
                      "stitch: player cell on west visible") &&
           ok;
      ok = assertTrue(tileAt(west, mapW - 1, 4)->isExplored,
                      "stitch: player cell on west explored") &&
           ok;
      // First tile of east map (world 8,4 → local 0,4)
      ok = assertTrue(tileAt(east, 0, 4)->isVisible,
                      "stitch: adjacent map tile visible") &&
           ok;
      ok = assertTrue(tileAt(east, 0, 4)->isExplored,
                      "stitch: adjacent map tile explored") &&
           ok;
      ok = assertTrue(tileAt(east, 1, 4)->isVisible,
                      "stitch: deeper tile on adjacent map visible") &&
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
