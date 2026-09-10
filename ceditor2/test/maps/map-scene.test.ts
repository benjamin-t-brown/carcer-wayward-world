import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import {
  buildMapScene,
  createMapScene,
  hitTestMapScene,
  isMapSceneBlockEditable,
  writeMapScene,
  type MapSceneWorldBounds,
} from '../../src/apps/maps/MapScene.js';
import { Viewport } from '../../src/apps/maps/canvas/Viewport.js';
import { translateTileGraphic } from '../../src/apps/maps/tools/tileGraphic.js';
import { MapGridTopology } from '../../src/core/domain/mapGrids/index.js';
import { MapDocument } from '../../src/core/domain/maps/index.js';

const DATABASE_URL = new URL('../../../src/assets/db/', import.meta.url);
const FOCUS_BOUNDS = { left: 0, top: 0, right: 56, bottom: 64 };

test('scene enumeration follows viewport bounds with one-cell overscan', () => {
  const documents = new Map<string, MapDocument>();
  const cells = Array.from({ length: 5 }, (_, y) =>
    Array.from({ length: 8 }, (_, x) => {
      const document = map(`map-${x}-${y}`);
      documents.set(document.name, document);
      return document.name;
    }),
  );
  const focus = documents.get('map-3-2')!;
  const grid = MapGridTopology.from({
    name: 'world',
    gridWidth: 8,
    gridHeight: 5,
    mapWidth: 2,
    mapHeight: 2,
    cells,
  });

  const scene = buildMapScene(focus, documents, [grid], {
    worldBounds: { left: 56, top: 0, right: 112, bottom: 64 },
  });

  assert.equal(scene.gridName, 'world');
  assert.equal(scene.blocks.length, 9);
  assert.deepEqual(scene.blocks.map((block) => block.document.name).sort(), [
    'map-3-1',
    'map-3-2',
    'map-3-3',
    'map-4-1',
    'map-4-2',
    'map-4-3',
    'map-5-1',
    'map-5-2',
    'map-5-3',
  ]);
  assert.ok(scene.blocks.every((block) => block.editable));
  assert.equal(scene.blocks.at(-1)?.document.name, focus.name);
});

test('massive grids inspect only visible and overscan cells', () => {
  const size = 1_000;
  const cells = Array.from({ length: size }, () => Array(size).fill(''));
  cells[500]![500] = 'focus';
  cells[500]![501] = 'east';
  const focus = map('focus');
  const east = map('east');
  const grid = MapGridTopology.from({
    name: 'massive',
    gridWidth: size,
    gridHeight: size,
    mapWidth: 2,
    mapHeight: 2,
    cells,
  });
  const originalMapNameAt = grid.mapNameAt.bind(grid);
  let inspectedCells = 0;
  grid.mapNameAt = (cellX, cellY) => {
    inspectedCells += 1;
    return originalMapNameAt(cellX, cellY);
  };

  const scene = buildMapScene(
    focus,
    new Map([
      ['focus', focus],
      ['east', east],
    ]),
    [grid],
    { worldBounds: FOCUS_BOUNDS },
  );

  assert.equal(inspectedCells, 9);
  assert.deepEqual(
    scene.blocks.map((block) => block.document.name),
    ['east', 'focus'],
  );
});

test('exclusive bounds choose exact boundary cells before overscan', () => {
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

  const focusOnly = buildMapScene(focus, documents, [grid], {
    worldBounds: FOCUS_BOUNDS,
    overscanCells: 0,
  });
  assert.deepEqual(
    focusOnly.blocks.map((block) => block.document.name),
    ['focus'],
  );

  const eastOnly = buildMapScene(focus, documents, [grid], {
    worldBounds: { left: 56, top: 0, right: 112, bottom: 64 },
    overscanCells: 0,
  });
  assert.deepEqual(
    eastOnly.blocks.map((block) => block.document.name),
    ['east'],
  );
  assert.deepEqual(hitTestMapScene(eastOnly, 56, 0, 0)?.cell, {
    documentId: 'east',
    layer: 0,
    index: 0,
  });
  assert.equal(hitTestMapScene(eastOnly, 112, 0, 0), undefined);
});

test('pan and zoom transforms produce equivalent world-bound scenes', () => {
  const focus = map('focus');
  const east = map('east');
  const grid = MapGridTopology.from({
    name: 'world',
    gridWidth: 2,
    gridHeight: 1,
    mapWidth: 2,
    mapHeight: 2,
    cells: [['focus', 'east']],
  });
  const documents = new Map([
    ['focus', focus],
    ['east', east],
  ]);
  const first = new Viewport({ x: -56, y: 0, scale: 1 });
  const second = new Viewport({ x: -112, y: 0, scale: 2 });
  const firstBounds = viewportBounds(first, 112, 64);
  const secondBounds = viewportBounds(second, 224, 128);

  assert.deepEqual(firstBounds, secondBounds);
  assert.deepEqual(
    buildMapScene(focus, documents, [grid], {
      worldBounds: firstBounds,
      overscanCells: 0,
    }).blocks.map((block) => block.document.name),
    buildMapScene(focus, documents, [grid], {
      worldBounds: secondBounds,
      overscanCells: 0,
    }).blocks.map((block) => block.document.name),
  );
});

test('standalone maps always remain a one-partition scene', () => {
  const focus = map('focus');
  const reusable = createMapScene();
  const scene = writeMapScene(
    reusable,
    focus,
    new Map([['focus', focus]]),
    [],
    {
      worldBounds: { left: 1_000, top: 1_000, right: 1_001, bottom: 1_001 },
    },
  );

  assert.equal(scene, reusable);
  assert.equal(scene.gridName, undefined);
  assert.equal(scene.blocks.length, 1);
  assert.equal(scene.blocks[0]?.document, focus);
  assert.equal(scene.blocks[0]?.focused, true);
  assert.equal(scene.blocks[0]?.editable, true);
});

test('scene writer reuses block objects and truncates prior output', () => {
  const focus = map('focus');
  const east = map('east');
  const grid = MapGridTopology.from({
    name: 'world',
    gridWidth: 2,
    gridHeight: 1,
    mapWidth: 2,
    mapHeight: 2,
    cells: [['focus', 'east']],
  });
  const documents = new Map([
    ['focus', focus],
    ['east', east],
  ]);
  const scene = buildMapScene(focus, documents, [grid], {
    worldBounds: { left: 0, top: 0, right: 112, bottom: 64 },
    overscanCells: 0,
  });
  const priorEastBlock = scene.blocks[0];

  writeMapScene(scene, focus, documents, [grid], {
    worldBounds: { left: 56, top: 0, right: 112, bottom: 64 },
    overscanCells: 0,
  });

  assert.equal(scene.blocks.length, 1);
  assert.equal(scene.blocks[0], priorEastBlock);
  assert.equal(scene.blocks[0]?.document.name, 'east');
});

test('retained hits stay stable when reusable scene blocks are rewritten', () => {
  const focus = map('focus');
  const east = map('east');
  const grid = MapGridTopology.from({
    name: 'world',
    gridWidth: 2,
    gridHeight: 1,
    mapWidth: 2,
    mapHeight: 2,
    cells: [['focus', 'east']],
  });
  const documents = new Map([
    ['focus', focus],
    ['east', east],
  ]);
  const scene = buildMapScene(focus, documents, [grid], {
    worldBounds: { left: 56, top: 0, right: 112, bottom: 64 },
    overscanCells: 0,
  });
  const retainedHit = hitTestMapScene(scene, 57, 1, 0);
  assert.ok(retainedHit);

  writeMapScene(scene, focus, documents, [grid], {
    worldBounds: FOCUS_BOUNDS,
    overscanCells: 0,
  });

  assert.equal(retainedHit.block.document.name, 'east');
  assert.equal(retainedHit.block.originX, 56);
  assert.equal(retainedHit.cell.documentId, 'east');
  assert.equal(scene.blocks[0]?.document.name, 'focus');
});

test('missing maps are skipped and incompatible partitions are read-only', () => {
  const focus = map('focus');
  const tall = map('tall', 28, 48);
  const wrongSize = MapDocument.from({
    ...focus.snapshot(),
    name: 'wrong-size',
    width: 1,
    tiles: { '0': [1, 0, 1, 1], '-1': [0, 0, 0, 0] },
  });
  const grid = MapGridTopology.from({
    name: 'world',
    gridWidth: 4,
    gridHeight: 1,
    mapWidth: 2,
    mapHeight: 2,
    cells: [['focus', 'missing', 'tall', 'wrong-size']],
  });
  const scene = buildMapScene(
    focus,
    new Map([
      ['focus', focus],
      ['tall', tall],
      ['wrong-size', wrongSize],
    ]),
    [grid],
    {
      worldBounds: { left: 0, top: 0, right: 224, bottom: 64 },
      overscanCells: 0,
    },
  );

  assert.deepEqual(
    scene.blocks.map((block) => [block.document.name, block.editable]),
    [
      ['tall', false],
      ['wrong-size', false],
      ['focus', true],
    ],
  );
  assert.equal(isMapSceneBlockEditable(scene.blocks[0]!, 0), false);
});

test('duplicate aliases use the focused map first placement as their anchor', () => {
  const focus = map('focus');
  const grid = MapGridTopology.from({
    name: 'duplicates',
    gridWidth: 3,
    gridHeight: 1,
    mapWidth: 2,
    mapHeight: 2,
    cells: [['focus', '', 'focus']],
  });
  const scene = buildMapScene(focus, new Map([['focus', focus]]), [grid], {
    worldBounds: { left: 0, top: 0, right: 168, bottom: 64 },
    overscanCells: 0,
  });

  assert.equal(scene.blocks.length, 2);
  assert.deepEqual(
    scene.blocks.map((block) => ({ x: block.originX, focused: block.focused })),
    [
      { x: 112, focused: false },
      { x: 0, focused: true },
    ],
  );
  assert.equal(hitTestMapScene(scene, 113, 1, 0)?.cell.documentId, 'focus');
});

test('cross-map graphics translate map-local tileset dictionary indexes', () => {
  const source = map('source');
  const target = MapDocument.from({
    ...source.snapshot(),
    name: 'target',
    tilesets: ['', 'other', 'terrain0'],
  });
  const missing = MapDocument.from({
    ...source.snapshot(),
    name: 'missing',
    tilesets: ['', 'other'],
  });

  assert.deepEqual(translateTileGraphic(source, target, 1, 42), [2, 42]);
  assert.equal(translateTileGraphic(source, missing, 1, 42), undefined);
  assert.equal(translateTileGraphic(source, target, 0, 42), undefined);
});

test('current Alinea data enumerates the visible continuous workspace', async () => {
  const [mapValues, gridValues] = await Promise.all([
    readJson(new URL('maps.json', DATABASE_URL)),
    readJson(new URL('map-grids.json', DATABASE_URL)),
  ]);
  assert.ok(Array.isArray(mapValues));
  assert.ok(Array.isArray(gridValues));
  const documents = mapValues.map((value, index) =>
    MapDocument.from(value, `maps[${index}]`),
  );
  const documentsByName = new Map(
    documents.map((document) => [document.name, document]),
  );
  const focused = documentsByName.get('alinea_outsideAlinea1');
  assert.ok(focused);
  const grids = gridValues.map((value, index) =>
    MapGridTopology.from(value, `mapGrids[${index}]`),
  );

  const scene = buildMapScene(focused, documentsByName, grids, {
    worldBounds: {
      left: 0,
      top: 0,
      right: focused.width * focused.spriteWidth,
      bottom: focused.height * focused.spriteHeight,
    },
  });

  assert.equal(scene.gridName, 'Alinea');
  assert.equal(scene.blocks.length, 9);
  assert.equal(scene.blocks.filter((block) => block.editable).length, 9);
  assert.equal(scene.blocks.at(-1)?.document.name, focused.name);
});

function viewportBounds(
  viewport: Viewport,
  width: number,
  height: number,
): MapSceneWorldBounds {
  return {
    left: viewport.screenToWorldX(0),
    top: viewport.screenToWorldY(0),
    right: viewport.screenToWorldX(width),
    bottom: viewport.screenToWorldY(height),
  };
}

function map(name: string, spriteWidth = 28, spriteHeight = 32): MapDocument {
  return MapDocument.from({
    name,
    label: name,
    type: 'TOWN',
    width: 2,
    height: 2,
    spriteWidth,
    spriteHeight,
    tilesets: ['', 'terrain0'],
    layers: [0, -1],
    tiles: {
      '0': [1, 0, 1, 1, 1, 2, 1, 3],
      '-1': [0, 0, 0, 0, 0, 0, 0, 0],
    },
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  });
}

async function readJson(url: URL): Promise<unknown> {
  return JSON.parse(await readFile(url, 'utf8')) as unknown;
}
