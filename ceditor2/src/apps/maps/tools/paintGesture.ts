import {
  CellPatchCommand,
  copyGraphic,
  graphicsEqual,
  type GraphicCell,
  type GraphicCellAccess,
  type TileGraphic,
} from '../history/cellPatches.js';

function validateCell(cell: GraphicCell): void {
  if (!cell.documentId.trim()) {
    throw new RangeError('Cell documentId must not be empty');
  }
  if (!Number.isInteger(cell.layer)) {
    throw new RangeError('Cell layer must be an integer');
  }
  if (!Number.isInteger(cell.index) || cell.index < 0) {
    throw new RangeError('Cell index must be a non-negative integer');
  }
}

function cellKey(cell: GraphicCell): string {
  return JSON.stringify([cell.documentId, cell.layer, cell.index]);
}

interface MutablePatch {
  cell: GraphicCell;
  before: TileGraphic;
  after: TileGraphic;
}

/**
 * Applies a pointer gesture immediately while retaining only the first before
 * graphic and latest after graphic for each visited cell.
 */
export class PaintGesture {
  private readonly patches = new Map<string, MutablePatch>();
  private closed = false;

  constructor(
    private readonly access: GraphicCellAccess,
    private readonly defaultGraphic: TileGraphic,
    private readonly defaultLabel: string,
  ) {}

  get changedCellCount(): number {
    let count = 0;
    for (const patch of this.patches.values()) {
      if (!graphicsEqual(patch.before, patch.after)) {
        count += 1;
      }
    }
    return count;
  }

  visit(cell: GraphicCell, graphic = this.defaultGraphic): boolean {
    this.assertOpen();
    validateCell(cell);
    const key = cellKey(cell);
    const existing = this.patches.get(key);
    const before =
      existing?.before ?? copyGraphic(this.access.getGraphic(cell));
    const after = copyGraphic(graphic);

    if (existing) {
      if (graphicsEqual(existing.after, after)) {
        return false;
      }
      existing.after = after;
    } else {
      if (graphicsEqual(before, after)) {
        return false;
      }
      this.patches.set(key, {
        cell: {
          documentId: cell.documentId,
          layer: cell.layer,
          index: cell.index,
        },
        before,
        after,
      });
    }

    this.access.setGraphic(cell, copyGraphic(after));
    return !graphicsEqual(before, after);
  }

  /** Close the gesture and return its single undoable command. */
  finish(label = this.defaultLabel): CellPatchCommand {
    this.assertOpen();
    this.closed = true;
    return new CellPatchCommand(label, [...this.patches.values()]);
  }

  /** Restore all touched cells and close the gesture without making a command. */
  cancel(): void {
    this.assertOpen();
    this.closed = true;
    const patches = [...this.patches.values()];
    for (let index = patches.length - 1; index >= 0; index -= 1) {
      const patch = patches[index]!;
      this.access.setGraphic(patch.cell, copyGraphic(patch.before));
    }
  }

  private assertOpen(): void {
    if (this.closed) {
      throw new Error('Paint gesture is already closed');
    }
  }
}

export function createPencilGesture(
  access: GraphicCellAccess,
  graphic: TileGraphic,
): PaintGesture {
  return new PaintGesture(access, copyGraphic(graphic), 'Pencil stroke');
}

export function createEraseGesture(
  access: GraphicCellAccess,
  emptyGraphic: TileGraphic = [0, 0],
): PaintGesture {
  return new PaintGesture(access, copyGraphic(emptyGraphic), 'Erase stroke');
}
