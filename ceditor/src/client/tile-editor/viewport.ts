/** Inclusive tile bounds that intersect the canvas, or null if fully off-screen. */
export type VisibleTileRange = {
  minX: number;
  maxX: number;
  minY: number;
  maxY: number;
};

/**
 * Tiles outside the canvas still cost a drawImage each. Clip to what can be
 * seen, retaining a small margin for sprites drawn beyond their tile bounds.
 */
export function getVisibleTileRange(args: {
  originX: number;
  originY: number;
  canvasWidth: number;
  canvasHeight: number;
  mapWidth: number;
  mapHeight: number;
  tileWidth: number;
  tileHeight: number;
  scale: number;
  marginTiles?: number;
}): VisibleTileRange | null {
  const {
    originX,
    originY,
    canvasWidth,
    canvasHeight,
    mapWidth,
    mapHeight,
    tileWidth,
    tileHeight,
    scale,
    marginTiles = 2,
  } = args;

  const stepX = tileWidth * scale;
  const stepY = tileHeight * scale;
  if (stepX <= 0 || stepY <= 0) {
    return { minX: 0, maxX: mapWidth - 1, minY: 0, maxY: mapHeight - 1 };
  }

  const minX = Math.max(0, Math.floor(-originX / stepX) - marginTiles);
  const maxX = Math.min(
    mapWidth - 1,
    Math.ceil((canvasWidth - originX) / stepX) + marginTiles,
  );
  const minY = Math.max(0, Math.floor(-originY / stepY) - marginTiles);
  const maxY = Math.min(
    mapHeight - 1,
    Math.ceil((canvasHeight - originY) / stepY) + marginTiles,
  );

  if (minX > maxX || minY > maxY) {
    return null;
  }
  return { minX, maxX, minY, maxY };
}
