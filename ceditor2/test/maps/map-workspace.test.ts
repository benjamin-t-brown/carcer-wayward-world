import assert from 'node:assert/strict';
import test from 'node:test';

import { MapWorkspace } from '../../src/apps/maps/MapWorkspace.js';
import { MapGridTopology } from '../../src/core/domain/mapGrids/index.js';
import { MapDocument } from '../../src/core/domain/maps/index.js';

test('workspace resolves negative and positive grid-world coordinates directly', () => {
  const west = map('west');
  const focus = map('focus');
  const east = map('east');
  const documents = new Map(
    [west, focus, east].map((document) => [document.name, document]),
  );
  const grid = MapGridTopology.from({
    name: 'world',
    gridWidth: 3,
    gridHeight: 1,
    mapWidth: 2,
    mapHeight: 2,
    cells: [['west', 'focus', 'east']],
  });
  const workspace = new MapWorkspace(focus, documents, [grid]);

  assert.equal(workspace.gridName, 'world');
  assert.deepEqual(workspace.resolve({ x: -1, y: 1 }, 0), {
    document: west,
    localX: 1,
    localY: 1,
    cell: { documentId: 'west', layer: 0, index: 3 },
  });
  assert.deepEqual(workspace.resolve({ x: 2, y: 0 }, 0), {
    document: east,
    localX: 0,
    localY: 0,
    cell: { documentId: 'east', layer: 0, index: 0 },
  });
  assert.equal(workspace.resolve({ x: 4, y: 0 }, 0), undefined);
});

test('workspace rejects missing layers and incompatible partitions', () => {
  const focus = map('focus');
  const wrong = map('wrong', 48);
  const grid = MapGridTopology.from({
    name: 'world',
    gridWidth: 2,
    gridHeight: 1,
    mapWidth: 2,
    mapHeight: 2,
    cells: [['focus', 'wrong']],
  });
  const workspace = new MapWorkspace(
    focus,
    new Map([
      ['focus', focus],
      ['wrong', wrong],
    ]),
    [grid],
  );
  assert.equal(workspace.resolve({ x: 0, y: 0 }, 2), undefined);
  assert.equal(workspace.resolve({ x: 2, y: 0 }, 0), undefined);
});

test('standalone workspace bounds checks the focused map', () => {
  const focus = map('focus');
  const workspace = new MapWorkspace(focus, new Map([['focus', focus]]), []);
  assert.equal(workspace.gridName, undefined);
  assert.equal(workspace.resolve({ x: 1, y: 1 }, 0)?.cell.index, 3);
  assert.equal(workspace.resolve({ x: -1, y: 0 }, 0), undefined);
});

function map(name: string, spriteWidth = 28): MapDocument {
  return MapDocument.from({
    name,
    width: 2,
    height: 2,
    spriteWidth,
    spriteHeight: 32,
    tilesets: ['', 'terrain'],
    layers: [0],
    tiles: { '0': [0, 0, 0, 0, 0, 0, 0, 0] },
  });
}
