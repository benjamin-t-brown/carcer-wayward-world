#pragma once

#include "model/instances/MapInstance.h"
#include "model/instances/World.h"
#include "model/templates/MapGrids.h"
#include "state/DatabaseInterface.h"
#include "state/StateManagerInterface.h"

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
  model::TileInstance* findTileAt(int worldX,
                                  int worldY,
                                  int mapLayerId = USE_WORLD_MAP_LAYER);
  ActiveMapMarker findMarker(const bmin::String& mapName,
                             const bmin::String& markerName);
};
} // namespace game