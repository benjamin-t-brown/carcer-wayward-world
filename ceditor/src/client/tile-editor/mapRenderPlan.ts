import type { CarcerMapTemplate, MapGridTemplate } from '../types/assets';
import type { MapGridPlacement } from '../utils/mapGridIndex';
import { getVisibleTileRange, type VisibleTileRange } from './viewport';

export const DEFAULT_PARTITION_OVERSCAN = 1;
export const DEFAULT_TILE_OVERSCAN = 2;

export interface MapRenderPlanOptions {
  readonly focusedMap: CarcerMapTemplate;
  readonly canvasWidth: number;
  readonly canvasHeight: number;
  /** Screen-space top-left of the focused map partition. */
  readonly focusOriginX: number;
  readonly focusOriginY: number;
  readonly scale: number;
  readonly mapsByName: ReadonlyMap<string, CarcerMapTemplate>;
  readonly placement?: MapGridPlacement | null;
  readonly partitionOverscan?: number;
  readonly tileOverscan?: number;
}

export interface PlannedMapPartition {
  readonly map: CarcerMapTemplate;
  readonly mapName: string;
  readonly cellX: number;
  readonly cellY: number;
  /** Grid-cell offset from the focused partition. */
  readonly offsetX: number;
  readonly offsetY: number;
  /** Screen-space top-left used directly for tile culling. */
  readonly originX: number;
  readonly originY: number;
  readonly focused: boolean;
  /** False for malformed partitions that cannot be edited seamlessly. */
  readonly editable: boolean;
  readonly visibleTiles: VisibleTileRange | null;
}

/** A bounded grid cell used for navigation/hit testing, including blank cells. */
export interface PlannedGridCell {
  readonly cellX: number;
  readonly cellY: number;
  readonly offsetX: number;
  readonly offsetY: number;
  readonly originX: number;
  readonly originY: number;
  readonly mapName: string;
  readonly map: CarcerMapTemplate | undefined;
  readonly focused: boolean;
  /** Loaded maps remain navigable; only immediate blank cells expose '+'. */
  readonly navigable: boolean;
}

export interface MapRenderPlan {
  readonly grid: MapGridTemplate | undefined;
  readonly partitions: readonly PlannedMapPartition[];
  readonly cells: readonly PlannedGridCell[];
  /** Number of grid cells inspected, including blank cells. */
  readonly visitedCellCount: number;
}

/**
 * Plan only the grid partitions intersecting the viewport plus bounded
 * overscan. The function reads map documents but never materializes tile
 * layers, so an offscreen partition has no layer-cache cost.
 */
export function buildMapRenderPlan(
  options: MapRenderPlanOptions,
): MapRenderPlan {
  const placement = options.placement;
  if (!placement) {
    return {
      grid: undefined,
      partitions: [standalonePartition(options)],
      cells: [standaloneCell(options)],
      visitedCellCount: 1,
    };
  }

  const { grid, cellX: anchorX, cellY: anchorY } = placement;
  const range = getVisibleGridCellRange(options, grid, anchorX, anchorY);
  if (!range) {
    return { grid, partitions: [], cells: [], visitedCellCount: 0 };
  }

  const partitions: PlannedMapPartition[] = [];
  const cells: PlannedGridCell[] = [];
  let focusedPartition: PlannedMapPartition | undefined;
  let visitedCellCount = 0;
  const slotWidth = grid.mapWidth * options.focusedMap.spriteWidth;
  const slotHeight = grid.mapHeight * options.focusedMap.spriteHeight;

  for (let cellY = range.minY; cellY <= range.maxY; cellY += 1) {
    const row = grid.cells[cellY];
    for (let cellX = range.minX; cellX <= range.maxX; cellX += 1) {
      visitedCellCount += 1;
      const isFocused = cellX === anchorX && cellY === anchorY;
      const mapName = isFocused
        ? options.focusedMap.name
        : (row?.[cellX]?.trim() ?? '');
      const map = isFocused
        ? options.focusedMap
        : mapName
          ? options.mapsByName.get(mapName)
          : undefined;
      const offsetX = cellX - anchorX;
      const offsetY = cellY - anchorY;
      const originX =
        options.focusOriginX + offsetX * slotWidth * options.scale;
      const originY =
        options.focusOriginY + offsetY * slotHeight * options.scale;
      cells.push({
        cellX,
        cellY,
        offsetX,
        offsetY,
        originX,
        originY,
        mapName,
        map,
        focused: isFocused,
        navigable:
          !isFocused &&
          (Boolean(mapName && map) ||
            ((!mapName || !map) &&
              Math.abs(offsetX) <= 1 &&
              Math.abs(offsetY) <= 1)),
      });
      if (!map) continue;

      const partition = createPartition({
        options,
        grid,
        map,
        cellX,
        cellY,
        offsetX,
        offsetY,
        originX,
        originY,
        focused: isFocused,
      });

      if (isFocused) {
        focusedPartition = partition;
      } else {
        partitions.push(partition);
      }
    }
  }

  // Keep the focused map topmost for both drawing and future reverse hit tests.
  if (focusedPartition) partitions.push(focusedPartition);
  return { grid, partitions, cells, visitedCellCount };
}

interface GridCellRange {
  readonly minX: number;
  readonly maxX: number;
  readonly minY: number;
  readonly maxY: number;
}

function getVisibleGridCellRange(
  options: MapRenderPlanOptions,
  grid: MapGridTemplate,
  anchorX: number,
  anchorY: number,
): GridCellRange | null {
  const { canvasWidth, canvasHeight, focusOriginX, focusOriginY, scale } =
    options;
  const slotWidth = grid.mapWidth * options.focusedMap.spriteWidth * scale;
  const slotHeight = grid.mapHeight * options.focusedMap.spriteHeight * scale;
  if (
    !positiveFinite(canvasWidth) ||
    !positiveFinite(canvasHeight) ||
    !Number.isFinite(focusOriginX) ||
    !Number.isFinite(focusOriginY) ||
    !positiveFinite(slotWidth) ||
    !positiveFinite(slotHeight) ||
    grid.gridWidth <= 0 ||
    grid.gridHeight <= 0
  ) {
    return null;
  }

  const overscan = nonNegativeInteger(
    options.partitionOverscan ?? DEFAULT_PARTITION_OVERSCAN,
  );
  const minOffsetX = Math.floor(-focusOriginX / slotWidth);
  const maxOffsetX = Math.ceil((canvasWidth - focusOriginX) / slotWidth) - 1;
  const minOffsetY = Math.floor(-focusOriginY / slotHeight);
  const maxOffsetY = Math.ceil((canvasHeight - focusOriginY) / slotHeight) - 1;
  const minX = Math.max(0, anchorX + minOffsetX - overscan);
  const maxX = Math.min(grid.gridWidth - 1, anchorX + maxOffsetX + overscan);
  const minY = Math.max(0, anchorY + minOffsetY - overscan);
  const maxY = Math.min(grid.gridHeight - 1, anchorY + maxOffsetY + overscan);

  return minX <= maxX && minY <= maxY ? { minX, maxX, minY, maxY } : null;
}

function standalonePartition(
  options: MapRenderPlanOptions,
): PlannedMapPartition {
  return createPartition({
    options,
    map: options.focusedMap,
    cellX: 0,
    cellY: 0,
    offsetX: 0,
    offsetY: 0,
    originX: options.focusOriginX,
    originY: options.focusOriginY,
    focused: true,
  });
}

function standaloneCell(options: MapRenderPlanOptions): PlannedGridCell {
  return {
    cellX: 0,
    cellY: 0,
    offsetX: 0,
    offsetY: 0,
    originX: options.focusOriginX,
    originY: options.focusOriginY,
    mapName: options.focusedMap.name,
    map: options.focusedMap,
    focused: true,
    navigable: false,
  };
}

function createPartition(args: {
  options: MapRenderPlanOptions;
  grid?: MapGridTemplate;
  map: CarcerMapTemplate;
  cellX: number;
  cellY: number;
  offsetX: number;
  offsetY: number;
  originX: number;
  originY: number;
  focused: boolean;
}): PlannedMapPartition {
  const {
    options,
    grid,
    map,
    cellX,
    cellY,
    offsetX,
    offsetY,
    originX,
    originY,
    focused,
  } = args;
  const editable =
    focused ||
    (!!grid &&
      map.width === grid.mapWidth &&
      map.height === grid.mapHeight &&
      map.spriteWidth === options.focusedMap.spriteWidth &&
      map.spriteHeight === options.focusedMap.spriteHeight);

  return {
    map,
    mapName: map.name,
    cellX,
    cellY,
    offsetX,
    offsetY,
    originX,
    originY,
    focused,
    editable,
    visibleTiles: getVisibleTileRange({
      originX,
      originY,
      canvasWidth: options.canvasWidth,
      canvasHeight: options.canvasHeight,
      mapWidth: map.width,
      mapHeight: map.height,
      tileWidth: map.spriteWidth,
      tileHeight: map.spriteHeight,
      scale: options.scale,
      marginTiles: nonNegativeInteger(
        options.tileOverscan ?? DEFAULT_TILE_OVERSCAN,
      ),
    }),
  };
}

function nonNegativeInteger(value: number): number {
  return Number.isFinite(value) ? Math.max(0, Math.floor(value)) : 0;
}

function positiveFinite(value: number): boolean {
  return Number.isFinite(value) && value > 0;
}
