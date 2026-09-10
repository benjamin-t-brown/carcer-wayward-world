import type { MapGridRecord } from '../../core/domain/mapGrids/index.js';

export interface MapGridCost {
  partitions: number;
  tiles: number;
  denseValues: number;
}

export function estimateMapGridCost(
  gridWidth: number,
  gridHeight: number,
  mapWidth: number,
  mapHeight: number,
  layerCount = 1,
): MapGridCost {
  const partitions = validCount(gridWidth) * validCount(gridHeight);
  const tiles = partitions * validCount(mapWidth) * validCount(mapHeight);
  return {
    partitions,
    tiles,
    denseValues: tiles * 2 * validCount(layerCount),
  };
}

export function matchesMapGridSearch(
  grid: MapGridRecord,
  searchTerm: string,
): boolean {
  const term = searchTerm.trim().toLocaleLowerCase();
  return (
    !term ||
    grid.name.toLocaleLowerCase().includes(term) ||
    (grid.label ?? '').toLocaleLowerCase().includes(term)
  );
}

export function createUniqueGridName(
  preferred: string,
  existing: Iterable<string>,
): string {
  const used = new Set(existing);
  const base = preferred.trim() || 'NEW_MAP_GRID';
  if (!used.has(base)) return base;
  for (let suffix = 2; ; suffix += 1) {
    const candidate = `${base}_${suffix}`;
    if (!used.has(candidate)) return candidate;
  }
}

function validCount(value: number): number {
  return Number.isSafeInteger(value) && value > 0 ? value : 0;
}
