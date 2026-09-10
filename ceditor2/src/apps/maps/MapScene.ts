import type { MapGridTopology } from '../../core/domain/mapGrids/index.js';
import type { MapDocument } from '../../core/domain/maps/index.js';
import type { GraphicCell } from './history/cellPatches.js';

export const DEFAULT_GRID_OVERSCAN_CELLS = 1;

/** Renderer-world bounds. Right and bottom are exclusive. */
export interface MapSceneWorldBounds {
  readonly left: number;
  readonly top: number;
  readonly right: number;
  readonly bottom: number;
}

export interface MapSceneOptions {
  readonly worldBounds: MapSceneWorldBounds;
  readonly overscanCells?: number;
}

/**
 * Scene blocks are reused by `writeMapScene`; consumers must not retain one
 * across later writes to the same scene.
 */
export interface MapSceneBlock {
  document: MapDocument;
  originX: number;
  originY: number;
  offsetX: number;
  offsetY: number;
  focused: boolean;
  editable: boolean;
}

export interface MapScene {
  gridName: string | undefined;
  readonly blocks: MapSceneBlock[];
}

export interface MapSceneHit {
  readonly block: MapSceneBlock;
  readonly cell: GraphicCell;
}

export function createMapScene(): MapScene {
  return { gridName: undefined, blocks: [] };
}

export function isMapSceneBlockEditable(
  block: MapSceneBlock,
  layer: number,
): boolean {
  return block.editable && block.document.hasLayer(layer);
}

/** Allocate a scene. Use `writeMapScene` in a render loop to reuse storage. */
export function buildMapScene(
  focused: MapDocument,
  documentsByName: ReadonlyMap<string, MapDocument>,
  grids: readonly MapGridTopology[],
  options: MapSceneOptions,
): MapScene {
  return writeMapScene(
    createMapScene(),
    focused,
    documentsByName,
    grids,
    options,
  );
}

/**
 * Write the visible portion of the focused map workspace into `out`.
 *
 * Grid scenes enumerate cells directly from viewport bounds. Runtime cost is
 * proportional to visible plus overscan cells, not total grid size. World
 * coordinates remain relative to the focused map's first row-major placement.
 */
export function writeMapScene(
  out: MapScene,
  focused: MapDocument,
  documentsByName: ReadonlyMap<string, MapDocument>,
  grids: readonly MapGridTopology[],
  options: MapSceneOptions,
): MapScene {
  for (const grid of grids) {
    const anchor = grid.placementOf(focused.name);
    if (!anchor || !matchesGridDimensions(grid, focused)) continue;

    out.gridName = grid.name;
    const range = visibleGridCellRange(
      grid,
      anchor.cellX,
      anchor.cellY,
      focused,
      options,
    );
    if (!range) {
      out.blocks.length = 0;
      return out;
    }

    let blockIndex = 0;
    let focusedVisible = false;
    for (let cellY = range.minY; cellY <= range.maxY; cellY += 1) {
      for (let cellX = range.minX; cellX <= range.maxX; cellX += 1) {
        const mapName = grid.mapNameAt(cellX, cellY);
        if (!mapName) continue;
        if (cellX === anchor.cellX && cellY === anchor.cellY) {
          focusedVisible = true;
          continue;
        }
        const document = documentsByName.get(mapName);
        if (!document) continue;
        writeBlock(
          out.blocks,
          blockIndex,
          document,
          cellX,
          cellY,
          anchor.cellX,
          anchor.cellY,
          focused,
          grid,
          false,
        );
        blockIndex += 1;
      }
    }

    // Draw and hit-test the focused partition last. This also makes malformed
    // duplicate aliases deterministic without hiding their other placement.
    if (focusedVisible) {
      writeBlock(
        out.blocks,
        blockIndex,
        focused,
        anchor.cellX,
        anchor.cellY,
        anchor.cellX,
        anchor.cellY,
        focused,
        grid,
        true,
      );
      blockIndex += 1;
    }
    out.blocks.length = blockIndex;
    return out;
  }

  out.gridName = undefined;
  writeStandaloneBlock(out.blocks, focused);
  out.blocks.length = 1;
  return out;
}

/** Resolve renderer-world coordinates to a real cell, checking topmost first. */
export function hitTestMapScene(
  scene: MapScene,
  worldX: number,
  worldY: number,
  layer: number,
): MapSceneHit | undefined {
  if (!Number.isFinite(worldX) || !Number.isFinite(worldY)) return undefined;

  for (
    let blockIndex = scene.blocks.length - 1;
    blockIndex >= 0;
    blockIndex -= 1
  ) {
    const block = scene.blocks[blockIndex]!;
    const document = block.document;
    const tileX = Math.floor((worldX - block.originX) / document.spriteWidth);
    const tileY = Math.floor((worldY - block.originY) / document.spriteHeight);
    const index = document.indexAt(tileX, tileY);
    if (index === undefined) continue;
    return {
      // Scene blocks are frame-reused; hits can survive into hover/selection
      // state and therefore need their own stable block snapshot.
      block: { ...block },
      cell: { documentId: document.name, layer, index },
    };
  }
  return undefined;
}

interface GridCellRange {
  readonly minX: number;
  readonly maxX: number;
  readonly minY: number;
  readonly maxY: number;
}

function visibleGridCellRange(
  grid: MapGridTopology,
  anchorX: number,
  anchorY: number,
  focused: MapDocument,
  options: MapSceneOptions,
): GridCellRange | undefined {
  const { left, top, right, bottom } = options.worldBounds;
  if (
    !Number.isFinite(left) ||
    !Number.isFinite(top) ||
    !Number.isFinite(right) ||
    !Number.isFinite(bottom) ||
    right <= left ||
    bottom <= top
  ) {
    return undefined;
  }

  const blockWidth = grid.mapWidth * focused.spriteWidth;
  const blockHeight = grid.mapHeight * focused.spriteHeight;
  const overscan = nonNegativeInteger(
    options.overscanCells ?? DEFAULT_GRID_OVERSCAN_CELLS,
  );
  const minX = Math.max(0, anchorX + Math.floor(left / blockWidth) - overscan);
  const maxX = Math.min(
    grid.gridWidth - 1,
    anchorX + Math.ceil(right / blockWidth) - 1 + overscan,
  );
  const minY = Math.max(0, anchorY + Math.floor(top / blockHeight) - overscan);
  const maxY = Math.min(
    grid.gridHeight - 1,
    anchorY + Math.ceil(bottom / blockHeight) - 1 + overscan,
  );

  return minX <= maxX && minY <= maxY ? { minX, maxX, minY, maxY } : undefined;
}

function writeBlock(
  blocks: MapSceneBlock[],
  index: number,
  document: MapDocument,
  cellX: number,
  cellY: number,
  anchorX: number,
  anchorY: number,
  focused: MapDocument,
  grid: MapGridTopology,
  isFocused: boolean,
): void {
  const block = blocks[index] ?? ({} as MapSceneBlock);
  block.document = document;
  block.originX = (cellX - anchorX) * grid.mapWidth * focused.spriteWidth;
  block.originY = (cellY - anchorY) * grid.mapHeight * focused.spriteHeight;
  block.offsetX = cellX - anchorX;
  block.offsetY = cellY - anchorY;
  block.focused = isFocused;
  block.editable =
    isFocused ||
    (matchesGridDimensions(grid, document) &&
      compatibleTileSize(focused, document));
  blocks[index] = block;
}

function writeStandaloneBlock(
  blocks: MapSceneBlock[],
  focused: MapDocument,
): void {
  const block = blocks[0] ?? ({} as MapSceneBlock);
  block.document = focused;
  block.originX = 0;
  block.originY = 0;
  block.offsetX = 0;
  block.offsetY = 0;
  block.focused = true;
  block.editable = true;
  blocks[0] = block;
}

function compatibleTileSize(left: MapDocument, right: MapDocument): boolean {
  return (
    left.spriteWidth === right.spriteWidth &&
    left.spriteHeight === right.spriteHeight
  );
}

function matchesGridDimensions(
  grid: MapGridTopology,
  document: MapDocument,
): boolean {
  return document.width === grid.mapWidth && document.height === grid.mapHeight;
}

function nonNegativeInteger(value: number): number {
  return Number.isFinite(value) ? Math.max(0, Math.floor(value)) : 0;
}
