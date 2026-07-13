#pragma once

#include "db/Database.h"
#include "model/instances/World.h"

namespace model {

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
bool isTileEffectivelySeeThrough(const TileInstance& tile, const db::Database& database);

bool doesTileBlockSight(const TileInstance& tile, const db::Database& database);

// See-through uses only the tile at (x,y) on map.tileLayerNumber.
// Empty cell (missing layer, empty tilesetName, or OOB index) → see-through.
bool isDestinationSeeThrough(const MapInstance& map,
                             int x,
                             int y,
                             const db::Database& database);

// Clear isVisible on all tiles, ray-cast through every cell in the vision octagon,
// OR into isExplored, then light opaque tiles that share an edge/corner with a visible
// see-through cell (continuous wall faces).
void updateMapVisibilityFromPlayer(MapInstance& map,
                                   int playerX,
                                   int playerY,
                                   const db::Database& database);

// Capture / apply explored bits (not visibility). Used by MapPersistence.
ExploredMapMask captureExploredMask(const MapInstance& map);
void applyExploredMask(MapInstance& map, const ExploredMapMask& mask);

} // namespace model
