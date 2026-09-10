import type { JsonObject } from '../../database/types.js';

/** Lossless representation of one record in map-grids.json. */
export type MapGridRecord = JsonObject & {
  name: string;
  label?: string;
  gridWidth: number;
  gridHeight: number;
  mapWidth: number;
  mapHeight: number;
  cells: string[][];
};

export interface MapGridCellPosition {
  cellX: number;
  cellY: number;
}

/** An assigned map block, positioned in grid-wide tile coordinates. */
export interface MapGridBlock extends MapGridCellPosition {
  gridName: string;
  mapName: string;
  originTileX: number;
  originTileY: number;
}

/** An in-bounds slot around an anchor; mapName is absent for an empty cell. */
export interface MapGridNeighborSlot extends MapGridCellPosition {
  gridName: string;
  offsetX: number;
  offsetY: number;
  originTileX: number;
  originTileY: number;
  mapName?: string;
}

export interface MapGridTileOrigin {
  tileX: number;
  tileY: number;
}

export interface MapGridTopologyIndex {
  readonly grids: readonly import('./MapGridTopology.js').MapGridTopology[];
  placementsOf(mapName: string): readonly MapGridBlock[];
  placementOf(mapName: string): MapGridBlock | undefined;
}
