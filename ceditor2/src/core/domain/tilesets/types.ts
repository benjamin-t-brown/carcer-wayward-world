import type { JsonObject } from '../../database/types.js';

export const TILE_STEP_SOUNDS = [
  { value: 0, label: 'Floor' },
  { value: 1, label: 'Grass' },
  { value: 2, label: 'Dirt' },
  { value: 3, label: 'Gravel' },
] as const;

export const TERRAIN_BORDER_TAGS = [
  'NONE',
  'GRASS',
  'DIRT',
  'WATER',
  'CAVE_FLOOR',
  'CAVE_WALL',
  'CAVE_WATER',
  'CLIFF',
  'SNOW',
] as const;

export type TileStepSound = 0 | 1 | 2 | 3;
export type StoredTileStepSound = number | string | null;
export type TerrainBorderTag = (typeof TERRAIN_BORDER_TAGS)[number] | string;

export type TileTerrainBorderMeta = JsonObject & {
  nw: TerrainBorderTag;
  ne: TerrainBorderTag;
  sw: TerrainBorderTag;
  se: TerrainBorderTag;
};

export type TileMetadata = JsonObject & {
  id: number;
  description?: string;
  stepSound?: StoredTileStepSound;
  isWalkable?: boolean;
  isSeeThrough?: boolean;
  isDoor?: boolean;
  isContainer?: boolean;
  tileTerrainBorderMeta?: TileTerrainBorderMeta;
};

export type TilesetTerrain = JsonObject & {
  primaryTerrain: TerrainBorderTag;
  secondaryTerrain: TerrainBorderTag;
  mode: number;
  startTileId: number;
};

/** Known editor fields intersect JsonObject so newer fields survive edits. */
export type TilesetRecord = JsonObject & {
  name: string;
  spriteBase?: string;
  imageWidth?: number;
  imageHeight?: number;
  tileWidth?: number;
  tileHeight?: number;
  terrain?: TilesetTerrain;
  tiles?: TileMetadata[];
};
