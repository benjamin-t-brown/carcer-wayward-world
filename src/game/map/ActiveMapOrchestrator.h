#pragma once

#include "model/instances/MapInstance.h"
#include "model/instances/World.h"
#include "model/templates/MapGrids.h"
#include <string_view>

namespace game {

struct ActiveMapLoc {
  int x = 0;
  int y = 0;
};

class ActiveMapOrchestrator {
  // can assume this exists, since it will load from the db, or this
  // class will throw if it doesn't exist.
  const model::MapGridTemplate* grid;
  model::World* world;

  static constexpr int USE_WORLD_MAP_LAYER = 9999;

public:
  ActiveMapOrchestrator(model::World& world);
  ActiveMapOrchestrator(model::World& world, const bmin::String& gridName);
  ~ActiveMapOrchestrator() = default;

  void loadMapGrid(const bmin::String& gridName);

  // return the relative position of the top left that this map
  // for a 3x3 grid, the top left would be (0,0), the top map would be (mapWidth, 0), then
  // (mapWidth * 2, 0), etc.  If the map name is not in the grid, return (-1, -1)
  ActiveMapLoc getMapOffset(const bmin::String& mapName) const;

  // convert from the active map coordinate system to the instance coordinate system
  // assume the top left of the grid is (0,0)
  ActiveMapLoc activeMapCoordToInstanceCoord(const ActiveMapLoc& activeMapLoc) const;
  ActiveMapLoc instanceCoordToActiveMapCoord(const ActiveMapLoc& instanceCoord) const;

  ActiveMapLoc getTotalWidthHeight() const;

  model::MapInstance* getMapInstanceFromCharacterId(
      const bmin::String& characterId, int mapLayerId = USE_WORLD_MAP_LAYER) const;
  model::MapInstance* getMapInstanceAt(const ActiveMapLoc& activeMapLoc,
                                       int mapLayerId = USE_WORLD_MAP_LAYER) const;

  model::CharacterInstance* findCharacterById(const bmin::String& characterId,
                                              int mapLayerId = USE_WORLD_MAP_LAYER) const;
  model::CharacterInstance* findCharacterAt(const ActiveMapLoc& activeMapLoc,
                                            int mapLayerId = USE_WORLD_MAP_LAYER) const;
  model::TileInstance* findTileAt(const ActiveMapLoc& activeMapLoc,
                                  int mapLayerId = USE_WORLD_MAP_LAYER) const;
  model::MapInstance* findMapByName(std::string_view mapName) const;
};
} // namespace game