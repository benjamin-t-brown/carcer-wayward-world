import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  TileMetadata,
  TileTerrainBorderMeta,
  TileTerrainBorderTag,
  TilesetTemplate,
} from '../types/assets';
import { EditorState, EditorStateMap, getEditorStateMap } from './editorState';
import { getTileList } from './editorEvents';

export const TERRAIN_TILESET_NAME = 'terrain_borders';

export function getTerrainTileset(
  tilesets: TilesetTemplate[]
): TilesetTemplate | undefined {
  return tilesets.find((t) => t.name === TERRAIN_TILESET_NAME);
}

export const PAINTABLE_TERRAIN_TAGS: TileTerrainBorderTag[] = [
  TileTerrainBorderTag.GRASS,
  TileTerrainBorderTag.DIRT,
  TileTerrainBorderTag.WATER,
];

export type TerrainLookup = Map<string, number>;

export function terrainMetaKey(meta: TileTerrainBorderMeta): string {
  return `${meta.nw}|${meta.ne}|${meta.sw}|${meta.se}`;
}

export function buildTerrainLookup(tileset: TilesetTemplate): TerrainLookup {
  const lookup: TerrainLookup = new Map();
  for (const tile of tileset.tiles) {
    if (!tile.tileTerrainBorderMeta) {
      continue;
    }
    lookup.set(terrainMetaKey(tile.tileTerrainBorderMeta), tile.id);
  }
  return lookup;
}

export function findInteriorTileId(
  lookup: TerrainLookup,
  tag: TileTerrainBorderTag
): number | undefined {
  return lookup.get(`${tag}|${tag}|${tag}|${tag}`);
}

export function getPaintableTerrainTags(
  lookup: TerrainLookup
): TileTerrainBorderTag[] {
  return PAINTABLE_TERRAIN_TAGS.filter(
    (tag) => findInteriorTileId(lookup, tag) !== undefined
  );
}

/**
 * Grass/dirt border tiles in terrain_borders use WATER in meta for the exterior
 * edge (see scratch terrain generator), not NONE. Empty map vertices are NONE, so
 * coerce them for lookup when painting that terrain.
 */
export function coerceCornersForTerrainResolve(
  corners: TileTerrainBorderMeta,
  paintTag: TileTerrainBorderTag
): TileTerrainBorderMeta {
  const mapVertex = (tag: TileTerrainBorderTag): TileTerrainBorderTag => {
    if (tag !== TileTerrainBorderTag.NONE) {
      return tag;
    }
    if (
      paintTag === TileTerrainBorderTag.GRASS ||
      paintTag === TileTerrainBorderTag.DIRT
    ) {
      return TileTerrainBorderTag.WATER;
    }
    return TileTerrainBorderTag.NONE;
  };
  return {
    nw: mapVertex(corners.nw),
    ne: mapVertex(corners.ne),
    sw: mapVertex(corners.sw),
    se: mapVertex(corners.se),
  };
}

function countMatchingCorners(
  meta: TileTerrainBorderMeta,
  corners: TileTerrainBorderMeta
): number {
  let count = 0;
  if (meta.nw === corners.nw) count++;
  if (meta.ne === corners.ne) count++;
  if (meta.sw === corners.sw) count++;
  if (meta.se === corners.se) count++;
  return count;
}

function isAllowedFallbackTile(
  meta: TileTerrainBorderMeta,
  paintTag: TileTerrainBorderTag
): boolean {
  const allowed = new Set<TileTerrainBorderTag>([
    paintTag,
    TileTerrainBorderTag.NONE,
  ]);
  if (
    paintTag === TileTerrainBorderTag.GRASS ||
    paintTag === TileTerrainBorderTag.DIRT
  ) {
    allowed.add(TileTerrainBorderTag.WATER);
  }
  return [meta.nw, meta.ne, meta.sw, meta.se].every((t) => allowed.has(t));
}

export function resolveTerrainTileId(
  lookup: TerrainLookup,
  corners: TileTerrainBorderMeta,
  tileset: TilesetTemplate,
  paintTag?: TileTerrainBorderTag
): number | null {
  const resolvedCorners = paintTag
    ? coerceCornersForTerrainResolve(corners, paintTag)
    : corners;

  const exact = lookup.get(terrainMetaKey(resolvedCorners));
  if (exact !== undefined) {
    return exact;
  }

  let bestId: number | null = null;
  let bestScore = -1;
  for (const tile of tileset.tiles) {
    if (!tile.tileTerrainBorderMeta) {
      continue;
    }
    if (paintTag && !isAllowedFallbackTile(tile.tileTerrainBorderMeta, paintTag)) {
      continue;
    }
    const score = countMatchingCorners(
      tile.tileTerrainBorderMeta,
      resolvedCorners
    );
    if (score > bestScore) {
      bestScore = score;
      bestId = tile.id;
    }
  }

  if (bestId !== null && bestScore < 4) {
    console.warn(
      'Terrain autotile: no exact match for',
      terrainMetaKey(resolvedCorners),
      'using fallback tile id',
      bestId
    );
  }
  return bestId;
}

export function vertexGridSize(mapData: CarcerMapTemplate): number {
  return (mapData.width + 1) * (mapData.height + 1);
}

function vertexIndex(
  mapData: CarcerMapTemplate,
  vx: number,
  vy: number
): number {
  return vy * (mapData.width + 1) + vx;
}

export function getVertex(
  grid: TileTerrainBorderTag[],
  mapData: CarcerMapTemplate,
  vx: number,
  vy: number
): TileTerrainBorderTag {
  if (
    vx < 0 ||
    vy < 0 ||
    vx > mapData.width ||
    vy > mapData.height
  ) {
    return TileTerrainBorderTag.NONE;
  }
  return grid[vertexIndex(mapData, vx, vy)] ?? TileTerrainBorderTag.NONE;
}

export function setVertex(
  grid: TileTerrainBorderTag[],
  mapData: CarcerMapTemplate,
  vx: number,
  vy: number,
  tag: TileTerrainBorderTag
): void {
  if (
    vx < 0 ||
    vy < 0 ||
    vx > mapData.width ||
    vy > mapData.height
  ) {
    return;
  }
  grid[vertexIndex(mapData, vx, vy)] = tag;
}

export function getTileCornersFromGrid(
  grid: TileTerrainBorderTag[],
  mapData: CarcerMapTemplate,
  x: number,
  y: number
): TileTerrainBorderMeta {
  return {
    nw: getVertex(grid, mapData, x, y),
    ne: getVertex(grid, mapData, x + 1, y),
    sw: getVertex(grid, mapData, x, y + 1),
    se: getVertex(grid, mapData, x + 1, y + 1),
  };
}

function preferVertexTag(
  current: TileTerrainBorderTag,
  incoming: TileTerrainBorderTag
): TileTerrainBorderTag {
  if (incoming !== TileTerrainBorderTag.NONE) {
    return incoming;
  }
  return current;
}

export function buildVertexGridFromMap(
  mapData: CarcerMapTemplate,
  level: number,
  terrainTileset: TilesetTemplate
): TileTerrainBorderTag[] {
  const size = vertexGridSize(mapData);
  const grid = new Array<TileTerrainBorderTag>(size).fill(
    TileTerrainBorderTag.NONE
  );
  const mapTiles = getTileList(mapData, level);
  const tileMetaById = new Map<number, TileMetadata>();
  for (const t of terrainTileset.tiles) {
    tileMetaById.set(t.id, t);
  }

  for (let y = 0; y < mapData.height; y++) {
    for (let x = 0; x < mapData.width; x++) {
      const ind = y * mapData.width + x;
      const mapTile = mapTiles[ind];
      if (mapTile.tilesetName !== TERRAIN_TILESET_NAME) {
        continue;
      }
      const meta = tileMetaById.get(mapTile.tileId)?.tileTerrainBorderMeta;
      if (!meta) {
        continue;
      }
      const corners: Array<[number, number, keyof TileTerrainBorderMeta]> = [
        [x, y, 'nw'],
        [x + 1, y, 'ne'],
        [x, y + 1, 'sw'],
        [x + 1, y + 1, 'se'],
      ];
      for (const [vx, vy, key] of corners) {
        const idx = vertexIndex(mapData, vx, vy);
        grid[idx] = preferVertexTag(grid[idx], meta[key]);
      }
    }
  }
  return grid;
}

export function getTerrainVertexGridForLevel(
  mapState: EditorStateMap,
  level: number
): TileTerrainBorderTag[] | undefined {
  return mapState.terrainVertexGridsByLevel?.[level];
}

export function ensureTerrainVertexGrid(
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  mapState: EditorStateMap,
  terrainTileset: TilesetTemplate,
  forceRebuild: boolean
): TileTerrainBorderTag[] {
  const level = editorState.currentLevel;
  const existing = getTerrainVertexGridForLevel(mapState, level);
  const needsRebuild =
    forceRebuild ||
    editorState.terrainGridDirty ||
    !existing ||
    existing.length !== vertexGridSize(mapData);

  if (needsRebuild) {
    if (!mapState.terrainVertexGridsByLevel) {
      mapState.terrainVertexGridsByLevel = {};
    }
    mapState.terrainVertexGridsByLevel[level] = buildVertexGridFromMap(
      mapData,
      level,
      terrainTileset
    );
    editorState.terrainGridDirty = false;
  }
  return mapState.terrainVertexGridsByLevel![level];
}

export function collectAffectedTileIndices(
  mapData: CarcerMapTemplate,
  x: number,
  y: number
): number[] {
  const indices: number[] = [];
  for (let dy = -1; dy <= 1; dy++) {
    for (let dx = -1; dx <= 1; dx++) {
      const tx = x + dx;
      const ty = y + dy;
      if (
        tx < 0 ||
        ty < 0 ||
        tx >= mapData.width ||
        ty >= mapData.height
      ) {
        continue;
      }
      indices.push(ty * mapData.width + tx);
    }
  }
  return indices;
}

export function buildTerrainPreviewGrid(
  grid: TileTerrainBorderTag[],
  mapData: CarcerMapTemplate,
  x: number,
  y: number,
  terrainTag: TileTerrainBorderTag
): TileTerrainBorderTag[] {
  const previewGrid = [...grid];
  setVertex(previewGrid, mapData, x, y, terrainTag);
  setVertex(previewGrid, mapData, x + 1, y, terrainTag);
  setVertex(previewGrid, mapData, x, y + 1, terrainTag);
  setVertex(previewGrid, mapData, x + 1, y + 1, terrainTag);
  return previewGrid;
}

export function previewTerrainCorners(
  grid: TileTerrainBorderTag[],
  mapData: CarcerMapTemplate,
  x: number,
  y: number,
  terrainTag: TileTerrainBorderTag
): TileTerrainBorderMeta {
  const previewGrid = buildTerrainPreviewGrid(
    grid,
    mapData,
    x,
    y,
    terrainTag
  );
  return getTileCornersFromGrid(previewGrid, mapData, x, y);
}

export interface TerrainPaintPreviewCell {
  x: number;
  y: number;
  tileId: number;
}

/** Tiles that would change if terrain were painted at (paintX, paintY). */
export function getTerrainPaintPreviewCells(
  mapData: CarcerMapTemplate,
  grid: TileTerrainBorderTag[],
  paintX: number,
  paintY: number,
  paintTag: TileTerrainBorderTag,
  lookup: TerrainLookup,
  terrainTileset: TilesetTemplate
): TerrainPaintPreviewCell[] {
  const previewGrid = buildTerrainPreviewGrid(
    grid,
    mapData,
    paintX,
    paintY,
    paintTag
  );
  const cells: TerrainPaintPreviewCell[] = [];
  for (const ind of collectAffectedTileIndices(mapData, paintX, paintY)) {
    const x = ind % mapData.width;
    const y = Math.floor(ind / mapData.width);
    const corners = getTileCornersFromGrid(previewGrid, mapData, x, y);
    const tileId = resolveTerrainTileId(
      lookup,
      corners,
      terrainTileset,
      paintTag
    );
    if (tileId !== null) {
      cells.push({ x, y, tileId });
    }
  }
  return cells;
}

export function applyTerrainToAffectedTiles(
  mapData: CarcerMapTemplate,
  mapTiles: CarcerMapTileTemplate[],
  grid: TileTerrainBorderTag[],
  affectedIndices: number[],
  lookup: TerrainLookup,
  terrainTileset: TilesetTemplate,
  paintTag: TileTerrainBorderTag
): void {
  for (const ind of affectedIndices) {
    const x = ind % mapData.width;
    const y = Math.floor(ind / mapData.width);
    const corners = getTileCornersFromGrid(grid, mapData, x, y);
    const tileId = resolveTerrainTileId(
      lookup,
      corners,
      terrainTileset,
      paintTag
    );
    if (tileId === null) {
      continue;
    }
    Object.assign(mapTiles[ind], {
      tilesetName: TERRAIN_TILESET_NAME,
      tileId,
    });
  }
}

export function paintTerrainAt(
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  mapState: EditorStateMap,
  terrainTileset: TilesetTemplate,
  lookup: TerrainLookup,
  x: number,
  y: number,
  terrainTag: TileTerrainBorderTag
): number[] {
  if (
    x < 0 ||
    y < 0 ||
    x >= mapData.width ||
    y >= mapData.height
  ) {
    return [];
  }

  const grid = ensureTerrainVertexGrid(
    mapData,
    editorState,
    mapState,
    terrainTileset,
    false
  );

  setVertex(grid, mapData, x, y, terrainTag);
  setVertex(grid, mapData, x + 1, y, terrainTag);
  setVertex(grid, mapData, x, y + 1, terrainTag);
  setVertex(grid, mapData, x + 1, y + 1, terrainTag);

  const affected = collectAffectedTileIndices(mapData, x, y);
  const mapTiles = getTileList(mapData, editorState.currentLevel);
  applyTerrainToAffectedTiles(
    mapData,
    mapTiles,
    grid,
    affected,
    lookup,
    terrainTileset,
    terrainTag
  );

  return affected;
}

export function invalidateTerrainVertexGridForMap(mapName: string): void {
  const mapState = getEditorStateMap(mapName);
  if (mapState) {
    delete mapState.terrainVertexGridsByLevel;
  }
}
