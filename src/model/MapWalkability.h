#pragma once

#include "db/Database.h"
#include "model/instances/World.h"
#include "model/templates/Tileset.h"
#include "bmin/DynArray.h"

namespace model {

const TileMetadata* findTileMetadata(const TilesetTemplate& tileset, int tileId);

const TileMetadata* resolveTileMetadata(const TileInstance& tile, const db::Database& database);

// Override wins when authored; else tileset isWalkable; empty tilesetName → true.
// Missing tileset/metadata with non-empty tilesetName → WARN and treat as walkable.
bool isTileEffectivelyWalkable(const TileInstance& tile, const db::Database& database);

// Closed door = tileset isDoor && !isWalkable. Ignores map walkability overrides.
bool isClosedDoorTile(const TileInstance& tile, const db::Database& database);

// Open door = tileset isDoor && isWalkable. Ignores map walkability overrides.
bool isOpenDoorTile(const TileInstance& tile, const db::Database& database);

// Capture / restore open-door tileIds (session travel persistence).
bmin::DynArray<OpenedDoorRecord> captureOpenedDoors(const MapInstance& map,
                                                    const db::Database& database);
void applyOpenedDoors(MapInstance& map, const bmin::DynArray<OpenedDoorRecord>& doors);

// Non-empty tiles at (x,y) across layers, sorted low→high layer.
void collectTilesAt(MapInstance& map, int x, int y, bmin::DynArray<TileInstance*>& out);
void collectTilesAt(const MapInstance& map,
                    int x,
                    int y,
                    bmin::DynArray<const TileInstance*>& out);

// Highest non-empty tile at (x,y) among layers <= map.tileLayerNumber.
// Ignores layers above the current layer. Returns nullptr if none / OOB.
const TileInstance* resolveTileToRender(const MapInstance& map, int x, int y);

// Non-empty tile on map.tileLayerNumber at (x,y), or nullptr if empty/missing/OOB.
const TileInstance* tileAtCurrentLayer(const MapInstance& map, int x, int y);
TileInstance* tileAtCurrentLayer(MapInstance& map, int x, int y);

// Walkability uses only the tile at (x,y) on map.tileLayerNumber.
// Empty cell (missing layer, empty tilesetName, or OOB index) → walkable.
bool isDestinationWalkable(const MapInstance& map, int x, int y, const db::Database& database);

// Closed door on map.tileLayerNumber at (x,y), or nullptr.
TileInstance* findClosedDoorAt(MapInstance& map,
                               int x,
                               int y,
                               const db::Database& database);

} // namespace model
