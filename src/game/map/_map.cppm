module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.game.map;
export import carcer.game.map.TileFields;
export import bmin.containers;
export import carcer.model.instances;
export import carcer.model.templates;
export import carcer.state;
export import carcer.db;
import bmin.string_interop;

export {

// --- from game/map/ActiveMapOrchestrator.h ---
namespace game {

struct ActiveMapLoc {
  int x = 0;
  int y = 0;
  bool valid = false;
};

struct ActiveMapMarker {
  bmin::String name;
  int x = 0;
  int y = 0;
  int layer = 0;
  bool valid = false;
};

class ActiveMapOrchestrator : public state::DatabaseInterface,
                              public state::StateManagerInterface {

  model::MapGridTemplate defaultGrid;
  // can assume this exists, since it will load from the db, or this
  // class will throw if it doesn't exist.
  const model::MapGridTemplate* grid = nullptr;
  static constexpr int USE_WORLD_MAP_LAYER = 9999;
  bmin::Map<int, model::MapInstance*> mapInstanceCache;

  const model::MapGridTemplate& requireGrid() const;
  int getMapLayerId(int mapLayerId) const;
  ActiveMapLoc findMapGridEntry(const bmin::String& mapName) const;
  model::MapInstance* getMapInstanceByName(const bmin::String& mapName);
  model::MapInstance* getMapInstanceAtGrid(int gridX, int gridY);

public:
  ActiveMapOrchestrator();
  ~ActiveMapOrchestrator() = default;

  void fetchMapGrid(const bmin::String& gridName);
  const model::MapGridTemplate& getMapGrid() const;

  // return the relative position of the top left that this map
  // for a 3x3 grid, the top left would be (0,0), the top map would be (mapWidth, 0), then
  // (mapWidth * 2, 0), etc.  If the map name is not in the grid, valid is false.
  ActiveMapLoc getMapOffset(const bmin::String& mapName) const;

  // convert from the active map coordinate system to the instance coordinate system
  // assume the top left of the grid is (0,0)
  ActiveMapLoc activeMapCoordToInstanceCoord(int worldX, int worldY) const;
  ActiveMapLoc instanceCoordToActiveMapCoord(const bmin::String& mapName,
                                             int mapX,
                                             int mapY) const;

  ActiveMapLoc getGridSize() const;
  ActiveMapLoc getTotalMapTilesSize() const;

  model::MapInstance* getDefaultMapInstance();
  model::MapInstance* getMapInstanceAt(int worldX, int worldY);

  model::CharacterInstance* findCharacterById(const bmin::String& characterId,
                                              int mapLayerId = USE_WORLD_MAP_LAYER);
  model::CharacterInstance* findCharacterAt(int worldX,
                                            int worldY,
                                            int mapLayerId = USE_WORLD_MAP_LAYER);
  bmin::DynArray<model::CharacterInstance*> findAllCharactersAt(
      int worldX, int worldY, int mapLayerId = USE_WORLD_MAP_LAYER);
  model::CharacterInstance* findCharacterAt(int worldX,
                                            int worldY,
                                            const bmin::String& excludeId,
                                            int mapLayerId = USE_WORLD_MAP_LAYER);
  model::TileInstance* findTileAt(int worldX,
                                  int worldY,
                                  int mapLayerId = USE_WORLD_MAP_LAYER);
  ActiveMapMarker findMarker(const bmin::String& mapName, const bmin::String& markerName);
};
} // namespace game

// --- from game/map/Camera.h ---
namespace game {

struct CameraPos {
  int camX = 0;
  int camY = 0;
};

// Center camera on target tile. Does not clamp — map may scroll past its edges
// so the target stays centered (empty space outside the map is allowed).
CameraPos computeCameraFollow(int targetTileX, int targetTileY, int viewW, int viewH);

} // namespace game

// --- from game/map/MapPathfinding.h ---
namespace game {

struct PathTile {
  int x = 0;
  int y = 0;
  /** Steps from the start tile (start itself is 0). */
  int dist = 0;
};

/**
 * Flood-fill tiles reachable by the given character within maxSteps.
 * 8-directional movement, step cost 1. Includes the start tile.
 * Other characters block tiles; characterId is ignored for occupancy.
 */
bmin::DynArray<PathTile> collectReachableTiles(model::ActiveMap& activeMap,
                                               int startX,
                                               int startY,
                                               int maxSteps,
                                               const bmin::String& characterId,
                                               const db::Database& database);

/** Same as above, using the character's current tile and id. */
bmin::DynArray<PathTile> collectReachableTiles(model::ActiveMap& activeMap,
                                               const model::CharacterInstance& character,
                                               int maxSteps,
                                               const db::Database& database);

bool isTileInReachableSet(const bmin::DynArray<PathTile>& reachable, int x, int y);

} // namespace game

// --- from game/map/MapPersistence.h ---
namespace game {

// Create a MapInstance for every map template in the database and store them on
// state.mapInstances.
void createMapInstances(state::State& state, const db::Database& database);

// Age tile fields on every MapInstance (and bump playerMovementCount).
void advanceWorldMovementTicks(state::State& state, int steps);

// Record a defeated map enemy on the MapInstance under its world position so it
// stays gone when entities are next hoisted into the active map.
void markMapCharacterDefeated(state::State& state,
                              const model::CharacterInstance& character);

// Resolve which map-grid to load for a travel destination (grid name, or the
// grid that contains the map, or a synthetic 1x1 grid for standalone maps).
bmin::String resolveGridIdForMapOrGrid(db::Database& database,
                                       const bmin::String& mapOrGridName);

} // namespace game

// --- from game/map/MapPickup.h ---
namespace game {

inline constexpr int PICKUP_PATH_RANGE = 4;

/** True when the active-map tile at (worldX, worldY) is effectively a container. */
bool isActiveMapTileContainer(model::ActiveMap& activeMap,
                              int worldX,
                              int worldY,
                              const db::Database& database);

/** Ground items the character can path to within maxSteps (excludes container tiles). */
bmin::DynArray<model::ItemInstance>
collectItemsWithinPickupRange(model::ActiveMap& activeMap,
                              const model::CharacterInstance& character,
                              int maxSteps,
                              const db::Database& database);

/** Items stored on a specific active-map tile (container contents or ground pile). */
bmin::DynArray<model::ItemInstance>
collectItemsAtActiveMapTile(const model::ActiveMap& activeMap, int worldX, int worldY);

} // namespace game

// --- from game/map/MapVision.h ---

namespace game {

// Half-extent of the vision radius (Chebyshev bound). Combined with a Manhattan cut,
// the lit area is an octagon rather than a square.
inline constexpr int kPlayerVisionBoxSize = 7;

// True if (dx, dy) from the player lies in the vision octagon.
inline bool isInPlayerVisionRange(int dx, int dy, int radius = kPlayerVisionBoxSize) {
  const auto adx = dx < 0 ? -dx : dx;
  const auto ady = dy < 0 ? -dy : dy;
  if (adx > radius || ady > radius) {
    return false;
  }
  // Cut square corners: |dx|+|dy| <= radius + radius/2 → regular-ish octagon.
  return adx + ady <= radius + radius / 2;
}

// Override wins when authored; else tileset isSeeThrough; empty tilesetName → true.
// Missing tileset/metadata with non-empty tilesetName → WARN and treat as see-through.
bool isTileEffectivelySeeThrough(const model::TileInstance& tile,
                                 const db::Database& database);

bool doesTileBlockSight(const model::TileInstance& tile, const db::Database& database);

// See-through uses only the tile at (x,y) on map.tileLayerNumber.
// Empty cell (missing layer, empty tilesetName, or OOB index) → see-through.
bool isDestinationSeeThrough(const model::MapInstance& map,
                             int x,
                             int y,
                             const db::Database& database);

// Clear isVisible on all tiles, ray-cast through every cell in the vision octagon,
// OR into isExplored, then light opaque tiles that share an edge/corner with a visible
// see-through cell (continuous wall faces).
void updateMapVisibilityFromPlayer(model::MapInstance& map,
                                   int playerX,
                                   int playerY,
                                   const db::Database& database);

// Union of vision from every party member on the map.
void updateMapVisibilityFromParty(model::MapInstance& map,
                                  const model::Player& player,
                                  const db::Database& database);

// Clear/rebuild visibility across every MapInstance in the active grid using
// party members on world.activeMap. Rays use world tile coordinates so vision
// crosses map-instance stitch edges within the active grid.
void updateActiveMapVisibilityFromParty(model::World& world,
                                        const model::Player& player,
                                        const db::Database& database);

// Clear grid visibility then light around a single world-coordinate observer
// (cross-instance raycast / wall-face lighting).
void updateActiveMapVisibilityFromPlayer(model::World& world,
                                         int worldX,
                                         int worldY,
                                         const db::Database& database);

// Capture / apply explored bits (not visibility). Used by MapPersistence.
model::ExploredMapMask captureExploredMask(const model::MapInstance& map);
void applyExploredMask(model::MapInstance& map, const model::ExploredMapMask& mask);

} // namespace game

// --- from game/map/MapWalkability.h ---
namespace game {

const model::TileMetadata* findTileMetadata(const model::TilesetTemplate& tileset,
                                            int tileId);

const model::TileMetadata* resolveTileMetadata(const model::TileInstance& tile,
                                               const db::Database& database);

// Override wins when authored; else tileset isWalkable; empty tilesetName → true.
// Missing tileset/metadata with non-empty tilesetName → WARN and treat as walkable.
bool isTileEffectivelyWalkable(const model::TileInstance& tile,
                               const db::Database& database);

// Override wins when authored; else tileset isContainer; empty/missing → false.
bool isTileEffectivelyContainer(const model::TileInstance& tile,
                                const db::Database& database);

// Closed door = tileset isDoor && !isWalkable. Ignores map walkability overrides.
bool isClosedDoorTile(const model::TileInstance& tile, const db::Database& database);

// Open door = tileset isDoor && isWalkable. Ignores map walkability overrides.
bool isOpenDoorTile(const model::TileInstance& tile, const db::Database& database);

// Capture / restore open-door tileIds (session travel persistence).
bmin::DynArray<model::OpenedDoorRecord> captureOpenedDoors(const model::MapInstance& map,
                                                           const db::Database& database);
void applyOpenedDoors(model::MapInstance& map,
                      const bmin::DynArray<model::OpenedDoorRecord>& doors);

// Non-empty tiles at (x,y) across layers, sorted low→high layer.
void collectTilesAt(model::MapInstance& map,
                    int x,
                    int y,
                    bmin::DynArray<model::TileInstance*>& out);
void collectTilesAt(const model::MapInstance& map,
                    int x,
                    int y,
                    bmin::DynArray<const model::TileInstance*>& out);

// Highest non-empty tile at (x,y) among layers <= map.tileLayerNumber.
// Ignores layers above the current layer. Returns nullptr if none / OOB.
const model::TileInstance*
resolveTileToRender(const model::MapInstance& map, int x, int y);

// Same notion MapView uses for "currently visible": resolveTileToRender + isVisible.
// Missing / empty render tile → not visible. Does not treat isExplored alone as visible.
bool isTileCurrentlyVisible(const model::MapInstance& map, int x, int y);

// Non-empty tile on map.tileLayerNumber at (x,y), or nullptr if empty/missing/OOB.
const model::TileInstance*
tileAtCurrentLayer(const model::MapInstance& map, int x, int y);
model::TileInstance* tileAtCurrentLayer(model::MapInstance& map, int x, int y);

// Walkability uses only the tile at (x,y) on map.tileLayerNumber.
// Empty cell (missing layer, empty tilesetName, or OOB index) → walkable.
bool isDestinationWalkable(const model::MapInstance& map,
                           int x,
                           int y,
                           const db::Database& database);

// Closed door on map.tileLayerNumber at (x,y), or nullptr.
model::TileInstance*
findClosedDoorAt(model::MapInstance& map, int x, int y, const db::Database& database);

} // namespace game

// --- from game/map/TileDistance.h ---
namespace game {

/** Chebyshev (king-move) distance between two tile coordinates. */
int chebyshevDistance(int x0, int y0, int x1, int y1);

/** True when the tiles are Chebyshev distance 1 (including diagonals). */
bool isChebyshevAdjacent(int x0, int y0, int x1, int y1);

} // namespace game

// --- from game/map/TileTriggers.h ---
namespace game {

// Party leader avatar (party[0]) on the active map, or nullptr.
// Independent of UI selection (selectedPartyMemberId).
model::CharacterInstance* findPartyAvatarOnActiveMap(model::ActiveMap& activeMap,
                                                     model::Player& player);

const model::CharacterInstance*
findPartyAvatarOnActiveMap(const model::ActiveMap& activeMap,
                           const model::Player& player);

// Resolve a world action-mode change: sets `world.actionMode`, the pending
// spell/caster context, and `world.actionAimTile` (combat caster if one is
// active, else the party leader avatar). Shared by the WorldSetActionMode
// action and any UI code that needs the same targeting logic without
// depending on the actions module (e.g. entering spell-aim mode from a
// spell-cast picker).
void resolveWorldActionMode(model::World& world,
                            const model::Player& player,
                            model::WorldActionMode mode,
                            const bmin::String& spellId = {},
                            const bmin::String& chId = {});

// Move existing party leader avatar to (x, y) world tiles, or create one from
// party[0] if missing. Returns nullptr if the party is empty.
// When database is set, caches template AI/faction fields on a newly created avatar.
model::CharacterInstance* placePartyAvatarAt(model::ActiveMap& activeMap,
                                             model::Player& player,
                                             int x,
                                             int y,
                                             const db::Database* database = nullptr);

// Character for drop/pickup placement: prefer the given character if present on
// the active map, otherwise the party leader avatar.
const model::CharacterInstance*
findDropCharacterOnActiveMap(const model::ActiveMap& activeMap,
                             const model::Player& player,
                             const bmin::String& characterId);
model::CharacterInstance* findDropCharacterOnActiveMap(model::ActiveMap& activeMap,
                                                       model::Player& player,
                                                       const bmin::String& characterId);

// After a successful step onto (x, y) local map coords: queue special event or travel.
void queueStepTriggersAt(state::Triggers& triggers,
                         const model::MapInstance& map,
                         int x,
                         int y);

// While standing on (x, y) local map coords: queue travel when requires action.
void queueActionTravelAtStanding(state::Triggers& triggers,
                                 const model::MapInstance& map,
                                 int x,
                                 int y);

// Console examine text: tile description, character labels, and item labels.
bmin::String formatExamineMessage(const model::MapInstance& map,
                                  const model::ActiveMap& activeMap,
                                  int worldX,
                                  int worldY,
                                  int localX,
                                  int localY,
                                  const db::Database& database);

} // namespace game

} // export
