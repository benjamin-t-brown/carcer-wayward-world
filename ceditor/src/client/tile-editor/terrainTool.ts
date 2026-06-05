import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  TileMetadata,
  TileTerrainBorderMeta,
  PAINTABLE_TERRAIN_BORDER_TAGS,
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

export const PAINTABLE_TERRAIN_TAGS = PAINTABLE_TERRAIN_BORDER_TAGS;

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
 * Empty map vertices are NONE. When resolving autotile corners, treat NONE as the
 * exterior terrain that borders the paint brush:
 * - Primary (e.g. GRASS) against void uses WATER (grass/water strip ids 0–12).
 * - Secondary (e.g. DIRT) against void uses primary (grass/dirt strip ids 16–28).
 * - WATER against void uses GRASS.
 */
export function inferExteriorVertexTag(
  paintTag: TileTerrainBorderTag,
  terrain?: TilesetTemplate['terrain']
): TileTerrainBorderTag {
  if (paintTag === TileTerrainBorderTag.NONE) {
    return TileTerrainBorderTag.NONE;
  }
  if (terrain) {
    const { primaryTerrain, secondaryTerrain } = terrain;
    if (paintTag === primaryTerrain) {
      return TileTerrainBorderTag.WATER;
    }
    if (paintTag === secondaryTerrain) {
      return primaryTerrain;
    }
  }
  if (
    paintTag === TileTerrainBorderTag.GRASS ||
    paintTag === TileTerrainBorderTag.DIRT
  ) {
    return TileTerrainBorderTag.WATER;
  }
  if (paintTag === TileTerrainBorderTag.WATER) {
    return TileTerrainBorderTag.GRASS;
  }
  return TileTerrainBorderTag.NONE;
}

export function coerceCornersForTerrainResolve(
  corners: TileTerrainBorderMeta,
  paintTag: TileTerrainBorderTag,
  terrain?: TilesetTemplate['terrain']
): TileTerrainBorderMeta {
  const exteriorTag = inferExteriorVertexTag(paintTag, terrain);
  const mapVertex = (tag: TileTerrainBorderTag): TileTerrainBorderTag => {
    if (tag !== TileTerrainBorderTag.NONE) {
      return tag;
    }
    return exteriorTag;
  };
  return {
    nw: mapVertex(corners.nw),
    ne: mapVertex(corners.ne),
    sw: mapVertex(corners.sw),
    se: mapVertex(corners.se),
  };
}

function getAllowedCornerTagsForPaint(
  paintTag: TileTerrainBorderTag,
  terrain?: TilesetTemplate['terrain']
): Set<TileTerrainBorderTag> {
  const allowed = new Set<TileTerrainBorderTag>([
    paintTag,
    TileTerrainBorderTag.NONE,
    inferExteriorVertexTag(paintTag, terrain),
  ]);
  if (terrain) {
    allowed.add(terrain.primaryTerrain);
    allowed.add(terrain.secondaryTerrain);
  }
  if (paintTag === TileTerrainBorderTag.GRASS) {
    allowed.add(TileTerrainBorderTag.WATER);
  }
  if (paintTag === TileTerrainBorderTag.WATER) {
    allowed.add(TileTerrainBorderTag.GRASS);
  }
  return allowed;
}

function countPaintTagCorners(
  meta: TileTerrainBorderMeta,
  paintTag: TileTerrainBorderTag
): number {
  let count = 0;
  if (meta.nw === paintTag) count++;
  if (meta.ne === paintTag) count++;
  if (meta.sw === paintTag) count++;
  if (meta.se === paintTag) count++;
  return count;
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
  paintTag: TileTerrainBorderTag,
  terrain?: TilesetTemplate['terrain']
): boolean {
  const allowed = getAllowedCornerTagsForPaint(paintTag, terrain);
  return [meta.nw, meta.ne, meta.sw, meta.se].every((t) => allowed.has(t));
}

export function resolveTerrainTileId(
  lookup: TerrainLookup,
  corners: TileTerrainBorderMeta,
  tileset: TilesetTemplate,
  paintTag?: TileTerrainBorderTag
): number | null {
  const resolvedCorners = paintTag
    ? coerceCornersForTerrainResolve(corners, paintTag, tileset.terrain)
    : corners;

  const exact = lookup.get(terrainMetaKey(resolvedCorners));
  if (exact !== undefined) {
    return exact;
  }

  let bestId: number | null = null;
  let bestScore = -1;
  let bestPaintTagScore = -1;
  for (const tile of tileset.tiles) {
    if (!tile.tileTerrainBorderMeta) {
      continue;
    }
    if (
      paintTag &&
      !isAllowedFallbackTile(tile.tileTerrainBorderMeta, paintTag, tileset.terrain)
    ) {
      continue;
    }
    const score = countMatchingCorners(
      tile.tileTerrainBorderMeta,
      resolvedCorners
    );
    const paintTagScore = paintTag
      ? countPaintTagCorners(tile.tileTerrainBorderMeta, paintTag)
      : 0;
    if (
      score > bestScore ||
      (score === bestScore && paintTagScore > bestPaintTagScore)
    ) {
      bestScore = score;
      bestPaintTagScore = paintTagScore;
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
