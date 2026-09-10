import assert from 'node:assert/strict';
import test from 'node:test';

import type {
  GraphicCell,
  GraphicCellAccess,
  TileGraphic,
} from '../../src/apps/maps/history/cellPatches.js';
import {
  captureGraphicBrush,
  floodFill,
  paintRectangle,
  stampGraphicBrush,
  type WorldCellResolver,
} from '../../src/apps/maps/tools/graphicRegionTools.js';

class Access implements GraphicCellAccess {
  readonly values = new Map<string, TileGraphic>();
  getGraphic(cell: GraphicCell): TileGraphic {
    return this.values.get(key(cell)) ?? [0, 0];
  }
  setGraphic(cell: GraphicCell, graphic: TileGraphic): void {
    this.values.set(key(cell), [...graphic] as TileGraphic);
  }
}

function workspace(width = 4, height = 2) {
  const access = new Access();
  const resolve: WorldCellResolver = ({ x, y }, layer) => {
    if (x < 0 || y < 0 || x >= width || y >= height) return undefined;
    const west = x < width / 2;
    const localX = west ? x : x - width / 2;
    return {
      documentId: west ? 'west' : 'east',
      layer,
      index: y * (width / 2) + localX,
    };
  };
  return { access, resolve };
}

test('rectangle painting crosses backing map boundaries in one command', () => {
  const { access, resolve } = workspace();
  const command = paintRectangle(
    access,
    resolve,
    0,
    { x: 1, y: 0 },
    { x: 2, y: 1 },
    [4, 9],
  );

  assert.equal(command.patches.length, 4);
  assert.deepEqual(
    new Set(command.patches.map(({ cell }) => cell.documentId)),
    new Set(['west', 'east']),
  );
  command.undo(access);
  assert.ok(
    command.patches.every(({ cell }) => access.getGraphic(cell)[0] === 0),
  );
  command.redo(access);
  assert.ok(
    command.patches.every(({ cell }) => access.getGraphic(cell)[1] === 9),
  );
});

test('a captured brush stamps its sparse resolvable footprint', () => {
  const { access, resolve } = workspace();
  access.setGraphic(resolve({ x: 0, y: 0 }, 1)!, [1, 10]);
  access.setGraphic(resolve({ x: 1, y: 0 }, 1)!, [1, 11]);
  const brush = captureGraphicBrush(
    access,
    resolve,
    1,
    { x: 0, y: 0 },
    { x: 1, y: 0 },
  );
  const command = stampGraphicBrush(access, resolve, 1, { x: 2, y: 1 }, brush);

  assert.equal(brush.width, 2);
  assert.equal(command.patches.length, 2);
  assert.deepEqual(access.getGraphic(resolve({ x: 2, y: 1 }, 1)!), [1, 10]);
  assert.deepEqual(access.getGraphic(resolve({ x: 3, y: 1 }, 1)!), [1, 11]);
});

test('four-way fill crosses partitions but does not cross another graphic', () => {
  const { access, resolve } = workspace(6, 2);
  access.setGraphic(resolve({ x: 4, y: 0 }, 0)!, [2, 7]);
  access.setGraphic(resolve({ x: 4, y: 1 }, 0)!, [2, 7]);
  const command = floodFill(access, resolve, 0, { x: 0, y: 0 }, [3, 8]);

  assert.equal(command.patches.length, 8);
  assert.deepEqual(access.getGraphic(resolve({ x: 3, y: 0 }, 0)!), [3, 8]);
  assert.deepEqual(access.getGraphic(resolve({ x: 4, y: 0 }, 0)!), [2, 7]);
});

test('fill safety limit rolls back already applied cells', () => {
  const { access, resolve } = workspace();
  assert.throws(
    () => floodFill(access, resolve, 0, { x: 0, y: 0 }, [1, 1], 2),
    /safety limit/,
  );
  for (let x = 0; x < 4; x += 1) {
    for (let y = 0; y < 2; y += 1) {
      assert.deepEqual(access.getGraphic(resolve({ x, y }, 0)!), [0, 0]);
    }
  }
});

test('mapped rectangle and semantic fill translate map-local dictionaries', () => {
  const { access, resolve } = workspace();
  for (let y = 0; y < 2; y += 1) {
    for (let x = 0; x < 4; x += 1) {
      const cell = resolve({ x, y }, 0)!;
      access.setGraphic(cell, [cell.documentId === 'west' ? 1 : 2, 5]);
    }
  }

  const fill = floodFill(
    access,
    resolve,
    0,
    { x: 0, y: 0 },
    (cell) => [cell.documentId === 'west' ? 3 : 4, 9],
    100,
    (cell, graphic, startCell, startGraphic) =>
      terrainName(cell, graphic) === terrainName(startCell, startGraphic),
  );
  assert.equal(fill.patches.length, 8);
  assert.deepEqual(access.getGraphic(resolve({ x: 1, y: 0 }, 0)!), [3, 9]);
  assert.deepEqual(access.getGraphic(resolve({ x: 2, y: 0 }, 0)!), [4, 9]);

  const rectangle = paintRectangle(
    access,
    resolve,
    0,
    { x: 1, y: 0 },
    { x: 2, y: 0 },
    (cell) => [cell.documentId === 'west' ? 5 : 6, 11],
  );
  assert.equal(rectangle.patches.length, 2);
  assert.deepEqual(access.getGraphic(resolve({ x: 1, y: 0 }, 0)!), [5, 11]);
  assert.deepEqual(access.getGraphic(resolve({ x: 2, y: 0 }, 0)!), [6, 11]);
});

function key(cell: GraphicCell): string {
  return `${cell.documentId}/${cell.layer}/${cell.index}`;
}

function terrainName(cell: GraphicCell, graphic: TileGraphic): string {
  const localTerrainIndex = cell.documentId === 'west' ? 1 : 2;
  return graphic[0] === localTerrainIndex ? `terrain/${graphic[1]}` : 'other';
}
