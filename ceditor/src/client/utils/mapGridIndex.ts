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

export function getGridAdjacentMaps(
  placement: MapGridPlacement,
  mapsByName: Record<string, CarcerMapTemplate>,
): GridAdjacentMap[] {
  const { grid, cellX, cellY } = placement;
  const adjacent: GridAdjacentMap[] = [];

  for (const { offsetX, offsetY } of NEIGHBOR_OFFSETS) {
    const neighborName = grid.cells[cellY + offsetY]?.[cellX + offsetX]?.trim();
    if (!neighborName) {
      continue;
    }
    const neighborMap = mapsByName[neighborName];
    if (!neighborMap) {
      continue;
    }
    adjacent.push({
      map: neighborMap,
      offsetX,
      offsetY,
    });
  }

  return adjacent;
}
