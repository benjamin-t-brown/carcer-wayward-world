import assert from 'node:assert/strict';
import test from 'node:test';
import { BoundedHistory } from '../../src/apps/maps/history/BoundedHistory.js';
import {
  CellPatchCommand,
  type GraphicCell,
  type GraphicCellAccess,
  type TileGraphic,
} from '../../src/apps/maps/history/cellPatches.js';
import {
  createEraseGesture,
  createPencilGesture,
} from '../../src/apps/maps/tools/paintGesture.js';

function key(cell: GraphicCell): string {
  return `${cell.documentId}:${cell.layer}:${cell.index}`;
}

class MemoryGraphics implements GraphicCellAccess {
  readonly values = new Map<string, TileGraphic>();
  writeCount = 0;

  getGraphic(cell: GraphicCell): TileGraphic {
    const graphic = this.values.get(key(cell));
    if (!graphic) {
      throw new Error(`Missing test cell ${key(cell)}`);
    }
    return graphic;
  }

  setGraphic(cell: GraphicCell, graphic: TileGraphic): void {
    this.writeCount += 1;
    this.values.set(key(cell), [graphic[0], graphic[1]]);
  }

  seed(cell: GraphicCell, graphic: TileGraphic): this {
    this.values.set(key(cell), [graphic[0], graphic[1]]);
    return this;
  }
}

const cellA: GraphicCell = { documentId: 'town', layer: 0, index: 4 };
const cellB: GraphicCell = { documentId: 'town', layer: 0, index: 5 };
const neighborCell: GraphicCell = {
  documentId: 'forest',
  layer: -1,
  index: 0,
};

test('a pencil gesture applies immediately and deduplicates revisited cells', () => {
  const graphics = new MemoryGraphics()
    .seed(cellA, [1, 10])
    .seed(cellB, [1, 11]);
  const gesture = createPencilGesture(graphics, [2, 20]);

  assert.equal(gesture.visit(cellA), true);
  assert.equal(gesture.visit(cellA), false);
  assert.equal(gesture.visit(cellB), true);
  assert.equal(graphics.writeCount, 2);

  const command = gesture.finish();
  assert.equal(command.label, 'Pencil stroke');
  assert.equal(command.patches.length, 2);
  assert.deepEqual(command.patches[0], {
    cell: cellA,
    before: [1, 10],
    after: [2, 20],
  });
  command.undo(graphics);
  assert.deepEqual(graphics.getGraphic(cellA), [1, 10]);
  assert.deepEqual(graphics.getGraphic(cellB), [1, 11]);
  command.redo(graphics);
  assert.deepEqual(graphics.getGraphic(cellA), [2, 20]);
  assert.deepEqual(graphics.getGraphic(cellB), [2, 20]);
});

test('one gesture across documents and layers records one history command', () => {
  const graphics = new MemoryGraphics()
    .seed(cellA, [1, 1])
    .seed(neighborCell, [3, 3]);
  const history = new BoundedHistory<GraphicCellAccess>();
  const gesture = createPencilGesture(graphics, [8, 8]);
  gesture.visit(cellA);
  gesture.visit(neighborCell);

  assert.equal(history.recordApplied(gesture.finish('Cross-map stroke')), true);
  assert.equal(history.undoDepth, 1);
  assert.equal(history.undo(graphics)?.label, 'Cross-map stroke');
  assert.deepEqual(graphics.getGraphic(cellA), [1, 1]);
  assert.deepEqual(graphics.getGraphic(neighborCell), [3, 3]);
  assert.equal(history.redo(graphics)?.label, 'Cross-map stroke');
  assert.deepEqual(graphics.getGraphic(neighborCell), [8, 8]);
});

test('a cell keeps its original before graphic and latest after graphic', () => {
  const graphics = new MemoryGraphics().seed(cellA, [1, 2]);
  const gesture = createPencilGesture(graphics, [2, 3]);
  gesture.visit(cellA);
  gesture.visit(cellA, [4, 5]);

  const command = gesture.finish();
  assert.deepEqual(command.patches[0]?.before, [1, 2]);
  assert.deepEqual(command.patches[0]?.after, [4, 5]);
  assert.equal(command.patches.length, 1);
});

test('erase uses the compact empty graphic and no-op gestures are not stored', () => {
  const graphics = new MemoryGraphics().seed(cellA, [7, 9]).seed(cellB, [0, 0]);
  const history = new BoundedHistory<GraphicCellAccess>();

  const erase = createEraseGesture(graphics);
  erase.visit(cellA);
  assert.deepEqual(graphics.getGraphic(cellA), [0, 0]);
  assert.equal(history.recordApplied(erase.finish()), true);

  const noOp = createEraseGesture(graphics);
  assert.equal(noOp.visit(cellB), false);
  assert.equal(history.recordApplied(noOp.finish()), false);
  assert.equal(history.undoDepth, 1);
});

test('cancel restores the gesture without creating a command', () => {
  const graphics = new MemoryGraphics().seed(cellA, [1, 2]);
  const gesture = createPencilGesture(graphics, [9, 9]);
  gesture.visit(cellA);
  gesture.cancel();

  assert.deepEqual(graphics.getGraphic(cellA), [1, 2]);
  assert.throws(() => gesture.visit(cellA), /already closed/);
  assert.throws(() => gesture.finish(), /already closed/);
});

test('bounded history drops oldest commands and supports redo branching', () => {
  const graphics = new MemoryGraphics().seed(cellA, [0, 0]);
  const history = new BoundedHistory<GraphicCellAccess>(2);
  const command = (before: TileGraphic, after: TileGraphic) =>
    new CellPatchCommand('paint', [{ cell: cellA, before, after }]);

  history.execute(command([0, 0], [1, 1]), graphics);
  history.execute(command([1, 1], [2, 2]), graphics);
  history.execute(command([2, 2], [3, 3]), graphics);
  assert.equal(history.undoDepth, 2);
  assert.deepEqual(graphics.getGraphic(cellA), [3, 3]);

  history.undo(graphics);
  history.undo(graphics);
  assert.deepEqual(graphics.getGraphic(cellA), [1, 1]);
  assert.equal(history.undo(graphics), undefined);
  assert.equal(history.redoDepth, 2);

  history.redo(graphics);
  assert.deepEqual(graphics.getGraphic(cellA), [2, 2]);
  history.execute(command([2, 2], [6, 6]), graphics);
  assert.equal(history.canRedo, false);
  assert.deepEqual(graphics.getGraphic(cellA), [6, 6]);
});

test('patch commands snapshot graphics without cloning a whole document', () => {
  const before: [number, number] = [1, 2];
  const after: [number, number] = [3, 4];
  const command = new CellPatchCommand('paint', [
    { cell: cellA, before, after },
  ]);
  before[0] = 99;
  after[1] = 99;

  assert.deepEqual(command.patches[0]?.before, [1, 2]);
  assert.deepEqual(command.patches[0]?.after, [3, 4]);
});

test('history rejects invalid bounds and gestures reject invalid cells', () => {
  assert.throws(() => new BoundedHistory(0), /positive integer/);
  const graphics = new MemoryGraphics();
  const gesture = createPencilGesture(graphics, [1, 1]);
  assert.throws(
    () => gesture.visit({ documentId: '', layer: 0, index: 0 }),
    /documentId/,
  );
  assert.throws(
    () => gesture.visit({ documentId: 'map', layer: 0, index: -1 }),
    /index/,
  );
});
