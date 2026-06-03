import { CarcerMapTileTemplate, TilesetTemplate } from '../types/assets';

/** Effective walkability: tile override, else tileset tile metadata. */
export function isMapTileWalkable(
  mapTile: CarcerMapTileTemplate,
  tilesets: TilesetTemplate[]
): boolean {
  const walkableOverride = mapTile.tileOverrides?.isWalkableOverride;
  if (walkableOverride !== undefined) {
    return walkableOverride;
  }

  if (!mapTile.tilesetName) {
    return true;
  }

  const tileset = tilesets.find((t) => t.name === mapTile.tilesetName);
  const tileMeta = tileset?.tiles[mapTile.tileId];
  if (!tileMeta) {
    return true;
  }

  return tileMeta.isWalkable !== false;
}
