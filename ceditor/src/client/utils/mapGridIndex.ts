import {
  CarcerMapTemplate,
  MapGridTemplate,
} from '../types/assets';

export interface MapGridPlacement {
  grid: MapGridTemplate;
  cellX: number;
  cellY: number;
}

export interface GridAdjacentMap {
  map: CarcerMapTemplate;
  /** Grid cell offset from the current cell. */
  offsetX: number;
  offsetY: number;
}

export interface GridAdjacentSlot {
  /** Grid cell offset from the current cell (magnitude up to the search radius). */
  offsetX: number;
  offsetY: number;
  cellX: number;
  cellY: number;
  mapName: string;
  map?: CarcerMapTemplate;
}

/** All grids that contain this map name (one placement per grid). */
export function findAllMapGridPlacements(
  mapName: string,
  grids: MapGridTemplate[],
): MapGridPlacement[] {
  const trimmed = mapName.trim();
  if (!trimmed) {
    return [];
  }

  const placements: MapGridPlacement[] = [];
  for (const grid of grids) {
    let foundInGrid = false;
    for (let cellY = 0; cellY < grid.cells.length && !foundInGrid; cellY++) {
      const row = grid.cells[cellY];
      if (!row) {
        continue;
      }
      for (let cellX = 0; cellX < row.length; cellX++) {
        if (row[cellX]?.trim() === trimmed) {
          placements.push({ grid, cellX, cellY });
          foundInGrid = true;
          break;
        }
      }
    }
  }
  return placements;
}

export function findMapGridPlacement(
  mapName: string,
  grids: MapGridTemplate[],
): MapGridPlacement | null {
  return findAllMapGridPlacements(mapName, grids)[0] ?? null;
}

/**
 * Every in-bounds grid cell within Chebyshev distance `radius` of the placement
 * (centre excluded). `radius` 1 is the eight immediate neighbours; higher values
 * add further rings. Each slot carries its map when the cell is assigned.
 */
export function getGridAdjacentSlots(
  placement: MapGridPlacement,
  mapsByName: Record<string, CarcerMapTemplate>,
  radius = 1,
): GridAdjacentSlot[] {
  const { grid, cellX, cellY } = placement;
  const r = Math.max(1, Math.floor(radius));
  const slots: GridAdjacentSlot[] = [];

  for (let offsetY = -r; offsetY <= r; offsetY++) {
    for (let offsetX = -r; offsetX <= r; offsetX++) {
      if (offsetX === 0 && offsetY === 0) {
        continue;
      }
      const neighborCellX = cellX + offsetX;
      const neighborCellY = cellY + offsetY;
      if (
        neighborCellY < 0 ||
        neighborCellY >= grid.gridHeight ||
        neighborCellX < 0 ||
        neighborCellX >= grid.gridWidth
      ) {
        continue;
      }
      const mapName = grid.cells[neighborCellY]?.[neighborCellX]?.trim() ?? '';
      const map = mapName ? mapsByName[mapName] : undefined;
      slots.push({
        offsetX,
        offsetY,
        cellX: neighborCellX,
        cellY: neighborCellY,
        mapName,
        map,
      });
    }
  }

  return slots;
}

export function isGridSlotEditable(slot: GridAdjacentSlot): boolean {
  return Boolean(slot.mapName && slot.map);
}

export interface GridBrushCellTarget {
  map: CarcerMapTemplate;
  tileIndex: number;
}

/**
 * Resolve a tile position given relative to `anchorMap`'s top-left (which may be
 * negative or past its edges) to the grid block that actually contains it,
 * walking whole grid cells. Returns null when it falls off the grid, onto an
 * unassigned/unloaded cell, or into a block too small for that local coord.
 * With no grid it just bounds-checks against `anchorMap` itself.
 */
export function resolveGridBrushCell(
  anchorMap: CarcerMapTemplate,
  localX: number,
  localY: number,
  mapGrids: MapGridTemplate[],
  mapsByName: Record<string, CarcerMapTemplate>,
): GridBrushCellTarget | null {
  if (
    localX >= 0 &&
    localX < anchorMap.width &&
    localY >= 0 &&
    localY < anchorMap.height
  ) {
    return { map: anchorMap, tileIndex: localY * anchorMap.width + localX };
  }

  const placement = findMapGridPlacement(anchorMap.name, mapGrids);
  if (!placement) {
    return null;
  }
  const cellW = placement.grid.mapWidth;
  const cellH = placement.grid.mapHeight;
  if (cellW <= 0 || cellH <= 0) {
    return null;
  }

  const cellDX = Math.floor(localX / cellW);
  const cellDY = Math.floor(localY / cellH);
  const cellX = placement.cellX + cellDX;
  const cellY = placement.cellY + cellDY;
  if (
    cellY < 0 ||
    cellY >= placement.grid.gridHeight ||
    cellX < 0 ||
    cellX >= placement.grid.gridWidth
  ) {
    return null;
  }

  const name = placement.grid.cells[cellY]?.[cellX]?.trim() ?? '';
  const map = name ? mapsByName[name] : undefined;
  if (!map) {
    return null;
  }
  const inX = localX - cellDX * cellW;
  const inY = localY - cellDY * cellH;
  if (inX < 0 || inX >= map.width || inY < 0 || inY >= map.height) {
    return null;
  }
  return { map, tileIndex: inY * map.width + inX };
}

/**
 * Whether the editor should show a click target on this slot: any cell holding a
 * map (so its "Open" rectangle appears wherever that map is drawn), plus the
 * immediate ring's empty cells (their "+" creates a map there).
 */
export function isGridSlotNavigable(slot: GridAdjacentSlot): boolean {
  return (
    isGridSlotEditable(slot) ||
    (Math.abs(slot.offsetX) <= 1 && Math.abs(slot.offsetY) <= 1)
  );
}

/**
 * Every assigned map whose grid cell is within Chebyshev distance `radius` of
 * the placement (centre excluded). Used to decide how much surrounding context
 * the editor paints around the current map — a rendering/perf knob.
 */
export function getGridMapsWithinRadius(
  placement: MapGridPlacement,
  mapsByName: Record<string, CarcerMapTemplate>,
  radius: number,
): GridAdjacentMap[] {
  return getGridAdjacentSlots(placement, mapsByName, radius)
    .filter(isGridSlotEditable)
    .map(({ map, offsetX, offsetY }) => ({
      map: map!,
      offsetX,
      offsetY,
    }));
}

export function assignMapToGridCell(
  grids: MapGridTemplate[],
  gridName: string,
  cellX: number,
  cellY: number,
  mapName: string,
): MapGridTemplate[] {
  return grids.map((grid) => {
    if (grid.name !== gridName) {
      return grid;
    }
    const cells = grid.cells.map((row, rowY) =>
      row.map((cell, colX) =>
        rowY === cellY && colX === cellX ? mapName : cell
      )
    );
    return { ...grid, cells };
  });
}

export function renameMapInGrids(
  grids: MapGridTemplate[],
  oldName: string,
  newName: string,
): MapGridTemplate[] {
  const trimmedOld = oldName.trim();
  const trimmedNew = newName.trim();
  if (!trimmedOld || !trimmedNew || trimmedOld === trimmedNew) {
    return grids;
  }

  return grids.map((grid) => {
    let changed = false;
    const cells = grid.cells.map((row) =>
      row.map((cell) => {
        if (cell?.trim() === trimmedOld) {
          changed = true;
          return trimmedNew;
        }
        return cell;
      })
    );
    return changed ? { ...grid, cells } : grid;
  });
}
