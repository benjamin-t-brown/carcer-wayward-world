#pragma once

#include "bmin/DynArray.h"
#include "db/Database.h"
#include "model/instances/World.h"
#include "model/templates/Tileset.h"


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
