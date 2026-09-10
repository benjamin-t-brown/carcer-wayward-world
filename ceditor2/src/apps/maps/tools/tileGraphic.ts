import type { MapDocument } from '../../../core/domain/maps/index.js';
import type { TileGraphic } from '../history/cellPatches.js';

/**
 * Convert a selection from one map-local tileset dictionary to another.
 * Tile IDs belong to the named tileset; the compact dictionary index does not.
 */
export function translateTileGraphic(
  source: MapDocument,
  target: MapDocument,
  sourceTilesetIndex: number,
  tileId: number,
): TileGraphic | undefined {
  if (
    !Number.isSafeInteger(sourceTilesetIndex) ||
    sourceTilesetIndex <= 0 ||
    !Number.isSafeInteger(tileId) ||
    tileId < 0
  )
    return undefined;
  const tilesetName = source.tilesetNames[sourceTilesetIndex];
  if (!tilesetName) return undefined;
  const targetTilesetIndex = target.tilesetNames.indexOf(tilesetName);
  return targetTilesetIndex > 0 ? [targetTilesetIndex, tileId] : undefined;
}
