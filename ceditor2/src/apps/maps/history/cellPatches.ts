/** A cell address can point into any map document and numeric layer. */
export interface GraphicCell {
  readonly documentId: string;
  readonly layer: number;
  readonly index: number;
}

/** Compact form used by maps.json: [tileset dictionary index, tile id]. */
export type TileGraphic = readonly [tilesetIndex: number, tileId: number];

/** The only document behavior required by graphic editing and history. */
export interface GraphicCellAccess {
  getGraphic(cell: GraphicCell): TileGraphic;
  setGraphic(cell: GraphicCell, graphic: TileGraphic): void;
}

export interface CellPatch {
  readonly cell: GraphicCell;
  readonly before: TileGraphic;
  readonly after: TileGraphic;
}

export interface HistoryCommand<Target> {
  readonly label: string;
  readonly isEmpty: boolean;
  undo(target: Target): void;
  redo(target: Target): void;
}

export function copyGraphic(graphic: TileGraphic): TileGraphic {
  return [graphic[0], graphic[1]];
}

export function graphicsEqual(left: TileGraphic, right: TileGraphic): boolean {
  return left[0] === right[0] && left[1] === right[1];
}

function copyCell(cell: GraphicCell): GraphicCell {
  return {
    documentId: cell.documentId,
    layer: cell.layer,
    index: cell.index,
  };
}

/** One compact, already-coalesced graphic edit command. */
export class CellPatchCommand implements HistoryCommand<GraphicCellAccess> {
  readonly patches: readonly CellPatch[];

  constructor(
    readonly label: string,
    patches: readonly CellPatch[],
  ) {
    this.patches = patches
      .filter((patch) => !graphicsEqual(patch.before, patch.after))
      .map((patch) => ({
        cell: copyCell(patch.cell),
        before: copyGraphic(patch.before),
        after: copyGraphic(patch.after),
      }));
  }

  get isEmpty(): boolean {
    return this.patches.length === 0;
  }

  undo(access: GraphicCellAccess): void {
    // Reverse order also makes this correct if a hand-built command happens to
    // contain duplicate cells. Gestures never produce duplicates.
    for (let index = this.patches.length - 1; index >= 0; index -= 1) {
      const patch = this.patches[index]!;
      access.setGraphic(patch.cell, copyGraphic(patch.before));
    }
  }

  redo(access: GraphicCellAccess): void {
    for (const patch of this.patches) {
      access.setGraphic(patch.cell, copyGraphic(patch.after));
    }
  }
}
