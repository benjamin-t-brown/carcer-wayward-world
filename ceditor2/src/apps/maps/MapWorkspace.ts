import type { MapGridTopology } from '../../core/domain/mapGrids/index.js';
import type { MapDocument } from '../../core/domain/maps/index.js';
import type { GraphicCell } from './history/cellPatches.js';
import type { WorldTilePoint } from './tools/graphicRegionTools.js';

export interface MapWorkspaceCell {
  readonly document: MapDocument;
  readonly cell: GraphicCell;
  readonly localX: number;
  readonly localY: number;
}

/**
 * Constant-time continuous tile resolver. It maps grid-world tile coordinates
 * back to the original finite map and local dense index without a merged map.
 */
export class MapWorkspace {
  readonly gridName: string | undefined;
  readonly #grid: MapGridTopology | undefined;
  readonly #anchorX: number;
  readonly #anchorY: number;

  constructor(
    readonly focused: MapDocument,
    private readonly documentsByName: ReadonlyMap<string, MapDocument>,
    grids: readonly MapGridTopology[],
  ) {
    const match = grids
      .map((grid) => ({ grid, anchor: grid.placementOf(focused.name) }))
      .find(
        ({ grid, anchor }) =>
          anchor !== undefined &&
          focused.width === grid.mapWidth &&
          focused.height === grid.mapHeight,
      );
    this.#grid = match?.grid;
    this.gridName = match?.grid.name;
    this.#anchorX = match?.anchor?.cellX ?? 0;
    this.#anchorY = match?.anchor?.cellY ?? 0;
  }

  resolve(point: WorldTilePoint, layer: number): MapWorkspaceCell | undefined {
    if (
      !Number.isSafeInteger(point.x) ||
      !Number.isSafeInteger(point.y) ||
      !Number.isSafeInteger(layer)
    )
      return undefined;

    if (!this.#grid) {
      return this.resolveInDocument(this.focused, point.x, point.y, layer);
    }
    const cellOffsetX = Math.floor(point.x / this.#grid.mapWidth);
    const cellOffsetY = Math.floor(point.y / this.#grid.mapHeight);
    const cellX = this.#anchorX + cellOffsetX;
    const cellY = this.#anchorY + cellOffsetY;
    const mapName = this.#grid.mapNameAt(cellX, cellY);
    if (!mapName) return undefined;
    const document = this.documentsByName.get(mapName);
    if (
      !document ||
      document.width !== this.#grid.mapWidth ||
      document.height !== this.#grid.mapHeight ||
      document.spriteWidth !== this.focused.spriteWidth ||
      document.spriteHeight !== this.focused.spriteHeight
    )
      return undefined;
    return this.resolveInDocument(
      document,
      point.x - cellOffsetX * this.#grid.mapWidth,
      point.y - cellOffsetY * this.#grid.mapHeight,
      layer,
    );
  }

  private resolveInDocument(
    document: MapDocument,
    localX: number,
    localY: number,
    layer: number,
  ): MapWorkspaceCell | undefined {
    if (!document.hasLayer(layer)) return undefined;
    const index = document.indexAt(localX, localY);
    return index === undefined
      ? undefined
      : {
          document,
          localX,
          localY,
          cell: { documentId: document.name, layer, index },
        };
  }
}
