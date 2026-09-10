import type {
  TerrainBorderTag,
  TileTerrainBorderMeta,
  TilesetRecord,
} from '../../../core/domain/tilesets/index.js';
import type { MapWorkspace, MapWorkspaceCell } from '../MapWorkspace.js';
import type {
  CellPatchCommand,
  GraphicCell,
  GraphicCellAccess,
  TileGraphic,
} from '../history/cellPatches.js';
import { PaintGesture } from './paintGesture.js';
import type { WorldTilePoint } from './graphicRegionTools.js';

export const TERRAIN_TILESET_NAME = 'terrain_borders';

const NONE = 'NONE';
const KNOWN_PAINTABLE_TAGS = [
  'GRASS',
  'DIRT',
  'WATER',
  'CAVE_FLOOR',
  'CAVE_WALL',
  'CAVE_WATER',
  'CLIFF',
  'SNOW',
] as const satisfies readonly TerrainBorderTag[];

/** Patterns supported by the legacy NONE-edge compatibility behavior. */
const COMPATIBILITY_PATTERNS = [
  'SSPP',
  'PPSS',
  'PSPS',
  'SPSP',
  'PSPP',
  'SPPP',
  'PPPS',
  'PPSP',
  'SSPS',
  'SSSP',
  'PSSS',
  'SPSS',
] as const;

const AFFECTED_CELLS = [
  { dx: 0, dy: 0, corners: ['nw', 'ne', 'sw', 'se'] },
  { dx: 0, dy: -1, corners: ['sw', 'se'] },
  { dx: 0, dy: 1, corners: ['nw', 'ne'] },
  { dx: -1, dy: 0, corners: ['ne', 'se'] },
  { dx: 1, dy: 0, corners: ['nw', 'sw'] },
  { dx: -1, dy: -1, corners: ['se'] },
  { dx: 1, dy: -1, corners: ['sw'] },
  { dx: -1, dy: 1, corners: ['ne'] },
  { dx: 1, dy: 1, corners: ['nw'] },
] as const;

type TerrainCorner = 'nw' | 'ne' | 'sw' | 'se';

export interface TerrainCorners {
  readonly nw: TerrainBorderTag;
  readonly ne: TerrainBorderTag;
  readonly sw: TerrainBorderTag;
  readonly se: TerrainBorderTag;
}

export class TerrainConfigurationError extends Error {
  constructor(message: string) {
    super(message);
    this.name = 'TerrainConfigurationError';
  }
}

/**
 * Immutable, database-revision-owned terrain index. Construct a new lookup
 * when tileset metadata changes; there is intentionally no module cache.
 */
export class TerrainMetadataLookup {
  readonly paintableTags: readonly TerrainBorderTag[];
  readonly #byCorners: ReadonlyMap<string, number>;
  readonly #byTileId: ReadonlyMap<number, TerrainCorners>;

  constructor(readonly tileset: TilesetRecord) {
    if (tileset.name !== TERRAIN_TILESET_NAME) {
      throw new TerrainConfigurationError(
        `Expected tileset "${TERRAIN_TILESET_NAME}", received "${tileset.name}"`,
      );
    }

    const exact = new Map<string, number>();
    const byTileId = new Map<number, TerrainCorners>();
    const uniformTags = new Set<TerrainBorderTag>();
    for (const tile of tileset.tiles ?? []) {
      const meta = copyCorners(tile.tileTerrainBorderMeta);
      if (!meta) continue;
      // Last metadata entry wins, matching the established editor behavior.
      exact.set(terrainCornersKey(meta), tile.id);
      byTileId.set(tile.id, meta);
      if (isUniform(meta) && meta.nw !== NONE) uniformTags.add(meta.nw);
    }

    const resolved = new Map(exact);
    addNoneCompatibilityAliases(resolved, exact, 'GRASS', 'DIRT');
    addNoneCompatibilityAliases(resolved, exact, 'DIRT', 'GRASS');
    addNoneCompatibilityAliases(resolved, exact, 'WATER', 'GRASS');

    const known = KNOWN_PAINTABLE_TAGS.filter((tag) => uniformTags.has(tag));
    const custom = [...uniformTags]
      .filter((tag) => !KNOWN_PAINTABLE_TAGS.includes(tag as never))
      .sort((left, right) => left.localeCompare(right));
    this.paintableTags = Object.freeze([...known, ...custom]);
    this.#byCorners = resolved;
    this.#byTileId = byTileId;
  }

  tileIdFor(corners: TerrainCorners): number | undefined {
    return this.#byCorners.get(terrainCornersKey(corners));
  }

  cornersFor(tileId: number): TerrainCorners | undefined {
    const corners = this.#byTileId.get(tileId);
    return corners ? { ...corners } : undefined;
  }
}

export function buildTerrainMetadataLookup(
  tilesets: readonly TilesetRecord[],
): TerrainMetadataLookup {
  const terrain = tilesets.find(
    (tileset) => tileset.name === TERRAIN_TILESET_NAME,
  );
  if (!terrain) {
    throw new TerrainConfigurationError(
      `Tileset "${TERRAIN_TILESET_NAME}" not found`,
    );
  }
  return new TerrainMetadataLookup(terrain);
}

export function terrainCornersKey(corners: TerrainCorners): string {
  return `${corners.nw}|${corners.ne}|${corners.sw}|${corners.se}`;
}

export function terrainTagLabel(tag: TerrainBorderTag): string {
  return tag
    .toLowerCase()
    .split('_')
    .map((part) => part.charAt(0).toUpperCase() + part.slice(1))
    .join(' ');
}

export interface TerrainPaintChange {
  readonly point: WorldTilePoint;
  readonly cell: GraphicCell;
  readonly graphic: TileGraphic;
}

export type TerrainPaintIssueReason =
  'missing-terrain-reference' | 'missing-variant';

export interface TerrainPaintIssue {
  readonly point: WorldTilePoint;
  readonly documentId: string;
  readonly reason: TerrainPaintIssueReason;
  readonly corners?: TerrainCorners;
}

export interface TerrainPaintPlan {
  readonly changes: readonly TerrainPaintChange[];
  readonly issues: readonly TerrainPaintIssue[];
}

/** Compute the center and eight-neighbor update without mutating map data. */
export function computeTerrainPaintPlan(
  access: GraphicCellAccess,
  workspace: MapWorkspace,
  lookup: TerrainMetadataLookup,
  layer: number,
  center: WorldTilePoint,
  tag: TerrainBorderTag,
): TerrainPaintPlan {
  assertPoint(center);
  if (!Number.isSafeInteger(layer)) {
    throw new RangeError('Terrain layer must be a safe integer');
  }
  const baseTileId = lookup.tileIdFor(uniformCorners(tag));
  if (baseTileId === undefined || tag === NONE) {
    throw new TerrainConfigurationError(
      `Base terrain tile not found for tag "${tag}"`,
    );
  }

  const centerCell = workspace.resolve(center, layer);
  if (!centerCell) return { changes: [], issues: [] };
  assertTerrainReference(centerCell);

  const changes: TerrainPaintChange[] = [];
  const issues: TerrainPaintIssue[] = [];
  const visited = new Set<string>();

  for (const affected of AFFECTED_CELLS) {
    const point = {
      x: center.x + affected.dx,
      y: center.y + affected.dy,
    };
    const resolved = workspace.resolve(point, layer);
    if (!resolved) continue;
    const cellKey = graphicCellKey(resolved.cell);
    if (visited.has(cellKey)) continue;
    visited.add(cellKey);

    const tilesetIndex = terrainTilesetIndex(resolved);
    if (tilesetIndex <= 0) {
      if (affected.dx === 0 && affected.dy === 0) {
        assertTerrainReference(resolved);
      }
      issues.push({
        point,
        documentId: resolved.document.name,
        reason: 'missing-terrain-reference',
      });
      continue;
    }

    const corners = currentTerrainCorners(access, resolved, lookup);
    for (const corner of affected.corners as readonly TerrainCorner[]) {
      corners[corner] = tag;
    }
    const tileId = lookup.tileIdFor(corners);
    if (tileId === undefined) {
      issues.push({
        point,
        documentId: resolved.document.name,
        reason: 'missing-variant',
        corners: { ...corners },
      });
      continue;
    }
    changes.push({
      point,
      cell: resolved.cell,
      graphic: [tilesetIndex, tileId],
    });
  }

  return { changes, issues };
}

/** Apply a precomputed terrain plan as one compact, undoable command. */
export function applyTerrainPaintPlan(
  access: GraphicCellAccess,
  plan: TerrainPaintPlan,
  label = 'Terrain paint',
): CellPatchCommand {
  const gesture = new PaintGesture(access, [0, 0], label);
  for (const change of plan.changes) {
    gesture.visit(change.cell, change.graphic);
  }
  return gesture.finish();
}

/** Compute and immediately apply one terrain dab. */
export function paintTerrainAt(
  access: GraphicCellAccess,
  workspace: MapWorkspace,
  lookup: TerrainMetadataLookup,
  layer: number,
  center: WorldTilePoint,
  tag: TerrainBorderTag,
): {
  readonly command: CellPatchCommand;
  readonly issues: readonly TerrainPaintIssue[];
} {
  const plan = computeTerrainPaintPlan(
    access,
    workspace,
    lookup,
    layer,
    center,
    tag,
  );
  return {
    command: applyTerrainPaintPlan(access, plan),
    issues: plan.issues,
  };
}

/** A continuous stroke coalesces repeated neighbor edits into one command. */
export class TerrainPaintStroke {
  readonly #gesture: PaintGesture;
  readonly #visitedCenters = new Set<string>();

  constructor(
    private readonly access: GraphicCellAccess,
    private readonly workspace: MapWorkspace,
    private readonly lookup: TerrainMetadataLookup,
    private readonly layer: number,
    private readonly tag: TerrainBorderTag,
  ) {
    // Validate the selected tag before a pointer stroke can mutate anything.
    if (lookup.tileIdFor(uniformCorners(tag)) === undefined || tag === NONE) {
      throw new TerrainConfigurationError(
        `Base terrain tile not found for tag "${tag}"`,
      );
    }
    this.#gesture = new PaintGesture(access, [0, 0], 'Terrain stroke');
  }

  paint(center: WorldTilePoint): TerrainPaintPlan {
    assertPoint(center);
    const key = `${center.x},${center.y}`;
    if (this.#visitedCenters.has(key)) return { changes: [], issues: [] };
    this.#visitedCenters.add(key);
    const plan = computeTerrainPaintPlan(
      this.access,
      this.workspace,
      this.lookup,
      this.layer,
      center,
      this.tag,
    );
    for (const change of plan.changes) {
      this.#gesture.visit(change.cell, change.graphic);
    }
    return plan;
  }

  finish(): CellPatchCommand {
    return this.#gesture.finish();
  }

  cancel(): void {
    this.#gesture.cancel();
  }
}

function currentTerrainCorners(
  access: GraphicCellAccess,
  resolved: MapWorkspaceCell,
  lookup: TerrainMetadataLookup,
): {
  nw: TerrainBorderTag;
  ne: TerrainBorderTag;
  sw: TerrainBorderTag;
  se: TerrainBorderTag;
} {
  const graphic = access.getGraphic(resolved.cell);
  if (resolved.document.tilesetNames[graphic[0]] !== TERRAIN_TILESET_NAME) {
    return mutableNoneCorners();
  }
  const corners = lookup.cornersFor(graphic[1]);
  return corners ? { ...corners } : mutableNoneCorners();
}

function terrainTilesetIndex(resolved: MapWorkspaceCell): number {
  return resolved.document.tilesetNames.indexOf(TERRAIN_TILESET_NAME);
}

function assertTerrainReference(resolved: MapWorkspaceCell): void {
  if (terrainTilesetIndex(resolved) <= 0) {
    throw new TerrainConfigurationError(
      `Map "${resolved.document.name}" does not reference tileset "${TERRAIN_TILESET_NAME}"`,
    );
  }
}

function copyCorners(
  meta: TileTerrainBorderMeta | undefined,
): TerrainCorners | undefined {
  return meta
    ? { nw: meta.nw, ne: meta.ne, sw: meta.sw, se: meta.se }
    : undefined;
}

function mutableNoneCorners(): {
  nw: TerrainBorderTag;
  ne: TerrainBorderTag;
  sw: TerrainBorderTag;
  se: TerrainBorderTag;
} {
  return { nw: NONE, ne: NONE, sw: NONE, se: NONE };
}

function uniformCorners(tag: TerrainBorderTag): TerrainCorners {
  return { nw: tag, ne: tag, sw: tag, se: tag };
}

function isUniform(corners: TerrainCorners): boolean {
  return (
    corners.nw === corners.ne &&
    corners.nw === corners.sw &&
    corners.nw === corners.se
  );
}

function addNoneCompatibilityAliases(
  resolved: Map<string, number>,
  exact: ReadonlyMap<string, number>,
  primary: TerrainBorderTag,
  fallback: TerrainBorderTag,
): void {
  for (const pattern of COMPATIBILITY_PATTERNS) {
    const source = cornersFromPattern(pattern, primary, fallback);
    const tileId = exact.get(terrainCornersKey(source));
    if (tileId === undefined) continue;
    const alias = cornersFromPattern(pattern, primary, NONE);
    const key = terrainCornersKey(alias);
    if (!resolved.has(key)) resolved.set(key, tileId);
  }
}

function cornersFromPattern(
  pattern: string,
  primary: TerrainBorderTag,
  secondary: TerrainBorderTag,
): TerrainCorners {
  const tags = [...pattern].map((value) =>
    value === 'P' ? primary : secondary,
  );
  return { nw: tags[0]!, ne: tags[1]!, sw: tags[2]!, se: tags[3]! };
}

function graphicCellKey(cell: GraphicCell): string {
  return `${cell.documentId}\u0000${cell.layer}\u0000${cell.index}`;
}

function assertPoint(point: WorldTilePoint): void {
  if (!Number.isSafeInteger(point.x) || !Number.isSafeInteger(point.y)) {
    throw new RangeError('Terrain coordinates must be safe integers');
  }
}
