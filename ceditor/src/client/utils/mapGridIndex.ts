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
  /** Grid cell offset from current: -1, 0, or 1 */
  offsetX: number;
  offsetY: number;
}

export interface GridAdjacentSlot {
  /** Grid cell offset from current: -1, 0, or 1 */
  offsetX: number;
  offsetY: number;
  cellX: number;
  cellY: number;
  mapName: string;
  map?: CarcerMapTemplate;
}

const NEIGHBOR_OFFSETS: ReadonlyArray<{ offsetX: number; offsetY: number }> =
  [
    { offsetX: -1, offsetY: -1 },
    { offsetX: 0, offsetY: -1 },
    { offsetX: 1, offsetY: -1 },
    { offsetX: -1, offsetY: 0 },
    { offsetX: 1, offsetY: 0 },
    { offsetX: -1, offsetY: 1 },
    { offsetX: 0, offsetY: 1 },
    { offsetX: 1, offsetY: 1 },
  ];

export function findMapGridPlacement(
  mapName: string,
  grids: MapGridTemplate[],
): MapGridPlacement | null {
  const trimmed = mapName.trim();
  if (!trimmed) {
    return null;
  }

  for (const grid of grids) {
    for (let cellY = 0; cellY < grid.cells.length; cellY++) {
      const row = grid.cells[cellY];
      if (!row) {
        continue;
      }
      for (let cellX = 0; cellX < row.length; cellX++) {
        if (row[cellX]?.trim() === trimmed) {
          return { grid, cellX, cellY };
        }
      }
    }
  }

  return null;
}

export function getGridAdjacentSlots(
  placement: MapGridPlacement,
  mapsByName: Record<string, CarcerMapTemplate>,
): GridAdjacentSlot[] {
  const { grid, cellX, cellY } = placement;
  const slots: GridAdjacentSlot[] = [];

  for (const { offsetX, offsetY } of NEIGHBOR_OFFSETS) {
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

  return slots;
}

export function isGridSlotEditable(slot: GridAdjacentSlot): boolean {
  return Boolean(slot.mapName && slot.map);
}

export function getGridAdjacentMaps(
  placement: MapGridPlacement,
  mapsByName: Record<string, CarcerMapTemplate>,
): GridAdjacentMap[] {
  return getGridAdjacentSlots(placement, mapsByName)
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
