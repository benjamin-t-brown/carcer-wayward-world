import { parseMapGridRecord } from './mapGridParser.js';
import type {
  MapGridBlock,
  MapGridCellPosition,
  MapGridNeighborSlot,
  MapGridRecord,
  MapGridTileOrigin,
} from './types.js';

function boundedInteger(value: number, limit: number): boolean {
  return Number.isSafeInteger(value) && value >= 0 && value < limit;
}

function normalizedMapName(value: string): string | undefined {
  const name = value.trim();
  return name || undefined;
}

/** Immutable lookup model for one rectangular map grid. */
export class MapGridTopology {
  readonly #record: MapGridRecord;
  readonly #blocks: readonly MapGridBlock[];
  readonly #blocksByMapName: ReadonlyMap<string, readonly MapGridBlock[]>;

  private constructor(record: MapGridRecord) {
    this.#record = record;

    const blocks: MapGridBlock[] = [];
    const mutableIndex = new Map<string, MapGridBlock[]>();
    for (let cellY = 0; cellY < record.gridHeight; cellY += 1) {
      for (let cellX = 0; cellX < record.gridWidth; cellX += 1) {
        const mapName = normalizedMapName(record.cells[cellY]![cellX]!);
        if (!mapName) {
          continue;
        }
        const block = Object.freeze({
          gridName: record.name,
          mapName,
          cellX,
          cellY,
          originTileX: cellX * record.mapWidth,
          originTileY: cellY * record.mapHeight,
        });
        blocks.push(block);
        const placements = mutableIndex.get(mapName) ?? [];
        placements.push(block);
        mutableIndex.set(mapName, placements);
      }
    }
    this.#blocks = Object.freeze(blocks);
    this.#blocksByMapName = new Map(
      [...mutableIndex].map(([mapName, placements]) => [
        mapName,
        Object.freeze(placements),
      ]),
    );
  }

  static from(value: unknown, path = 'mapGrid'): MapGridTopology {
    return new MapGridTopology(parseMapGridRecord(value, path));
  }

  get name(): string {
    return this.#record.name;
  }

  get gridWidth(): number {
    return this.#record.gridWidth;
  }

  get gridHeight(): number {
    return this.#record.gridHeight;
  }

  get mapWidth(): number {
    return this.#record.mapWidth;
  }

  get mapHeight(): number {
    return this.#record.mapHeight;
  }

  mapNameAt(cellX: number, cellY: number): string | undefined {
    if (!this.#contains(cellX, cellY)) {
      return undefined;
    }
    return normalizedMapName(this.#record.cells[cellY]![cellX]!);
  }

  originAt(cellX: number, cellY: number): MapGridTileOrigin | undefined {
    if (!this.#contains(cellX, cellY)) {
      return undefined;
    }
    return {
      tileX: cellX * this.mapWidth,
      tileY: cellY * this.mapHeight,
    };
  }

  blockAt(cellX: number, cellY: number): MapGridBlock | undefined {
    const mapName = this.mapNameAt(cellX, cellY);
    if (!mapName) {
      return undefined;
    }
    return this.#blocksByMapName
      .get(mapName)
      ?.find((block) => block.cellX === cellX && block.cellY === cellY);
  }

  blocks(): readonly MapGridBlock[] {
    return this.#blocks;
  }

  placementsOf(mapName: string): readonly MapGridBlock[] {
    const name = normalizedMapName(mapName);
    return name ? (this.#blocksByMapName.get(name) ?? []) : [];
  }

  /** The first row-major placement when a malformed grid repeats a map. */
  placementOf(mapName: string): MapGridBlock | undefined {
    return this.placementsOf(mapName)[0];
  }

  /**
   * Return every in-bounds slot in a Chebyshev radius, in row-major order.
   * Empty slots are included and have no mapName.
   */
  neighborSlots(
    anchor: MapGridCellPosition,
    radius = 1,
  ): readonly MapGridNeighborSlot[] {
    if (!this.#contains(anchor.cellX, anchor.cellY)) {
      return [];
    }
    const normalizedRadius = Number.isFinite(radius)
      ? Math.max(0, Math.floor(radius))
      : 0;
    const slots: MapGridNeighborSlot[] = [];
    for (
      let offsetY = -normalizedRadius;
      offsetY <= normalizedRadius;
      offsetY += 1
    ) {
      for (
        let offsetX = -normalizedRadius;
        offsetX <= normalizedRadius;
        offsetX += 1
      ) {
        if (offsetX === 0 && offsetY === 0) {
          continue;
        }
        const cellX = anchor.cellX + offsetX;
        const cellY = anchor.cellY + offsetY;
        const origin = this.originAt(cellX, cellY);
        if (!origin) {
          continue;
        }
        const mapName = this.mapNameAt(cellX, cellY);
        slots.push(
          Object.freeze({
            gridName: this.name,
            cellX,
            cellY,
            offsetX,
            offsetY,
            originTileX: origin.tileX,
            originTileY: origin.tileY,
            ...(mapName ? { mapName } : {}),
          }),
        );
      }
    }
    return Object.freeze(slots);
  }

  snapshot(): MapGridRecord {
    return structuredClone(this.#record);
  }

  #contains(cellX: number, cellY: number): boolean {
    return (
      boundedInteger(cellX, this.gridWidth) &&
      boundedInteger(cellY, this.gridHeight)
    );
  }
}
