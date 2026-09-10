import { MapGridTopology } from './MapGridTopology.js';
import { parseMapGridCollection } from './mapGridParser.js';
import type {
  MapGridBlock,
  MapGridTopologyIndex,
  MapGridRecord,
} from './types.js';

/** Build deterministic cross-grid lookups in source-grid, then row-major order. */
export function buildMapGridTopologyIndex(
  value: unknown,
  path = 'mapGrids',
): MapGridTopologyIndex {
  const records = parseMapGridCollection(value, path);
  const grids = Object.freeze(
    records.map((record, index) =>
      MapGridTopology.from(record, `${path}[${index}]`),
    ),
  );
  const mutablePlacements = new Map<string, MapGridBlock[]>();
  for (const grid of grids) {
    for (const block of grid.blocks()) {
      const placements = mutablePlacements.get(block.mapName) ?? [];
      placements.push(block);
      mutablePlacements.set(block.mapName, placements);
    }
  }
  const placements = new Map(
    [...mutablePlacements].map(([mapName, blocks]) => [
      mapName,
      Object.freeze(blocks),
    ]),
  );

  return Object.freeze({
    grids,
    placementsOf(mapName: string): readonly MapGridBlock[] {
      const name = mapName.trim();
      return name ? (placements.get(name) ?? []) : [];
    },
    placementOf(mapName: string): MapGridBlock | undefined {
      const name = mapName.trim();
      return name ? placements.get(name)?.[0] : undefined;
    },
  });
}

/** Convenience for callers that already parsed a database collection. */
export function buildMapGridTopologyIndexFromRecords(
  records: readonly MapGridRecord[],
  path = 'mapGrids',
): MapGridTopologyIndex {
  return buildMapGridTopologyIndex(records, path);
}
