import assert from 'node:assert/strict';
import test from 'node:test';
import { CarcerMapTemplate, MapGridTemplate } from '../client/types/assets';
import {
  assignMapToGridCell,
  findAllMapGridPlacements,
  findMapGridPlacement,
  getGridAdjacentSlots,
  getGridLayerSet,
  getGridMapsWithinRadius,
  isGridSlotEditable,
  isGridSlotNavigable,
  renameMapInGrids,
  resolveGridBrushCell,
} from '../client/utils/mapGridIndex';
import { getVisibleTileRange } from '../client/tile-editor/viewport';

function map(
  name: string,
  width = 4,
  height = 3,
  layers: number[] = [0],
): CarcerMapTemplate {
  return {
    name,
    label: name,
    type: 'TOWN',
    width,
    height,
    spriteWidth: 16,
    spriteHeight: 16,
    tilesets: [''],
    layers,
    tiles: {},
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };
}

function grid(
  name: string,
  cells: string[][],
  mapWidth = 4,
  mapHeight = 3,
): MapGridTemplate {
  return {
    name,
    label: name,
    gridWidth: cells[0]?.length ?? 0,
    gridHeight: cells.length,
    mapWidth,
    mapHeight,
    cells,
  };
}

test('placement lookup trims names, returns one placement per grid, and preserves grid order', () => {
  const first = grid('first', [
    [' center ', ''],
    ['', 'center'],
  ]);
  const second = grid('second', [['', 'center']]);

  const placements = findAllMapGridPlacements('center', [first, second]);
  assert.deepEqual(
    placements.map(({ grid: owner, cellX, cellY }) => [
      owner.name,
      cellX,
      cellY,
    ]),
    [
      ['first', 0, 0],
      ['second', 1, 0],
    ],
  );
  assert.equal(findMapGridPlacement('center', [first, second])?.grid, first);
  assert.deepEqual(findAllMapGridPlacements('   ', [first]), []);
});

test('adjacent slots use Chebyshev radius, exclude the center, and clamp to grid bounds', () => {
  const documentGrid = grid('world', [
    ['northWest', 'north', 'northEast', ''],
    ['west', 'center', 'east', 'farEast'],
    ['southWest', '', 'southEast', ''],
  ]);
  const center = findMapGridPlacement('center', [documentGrid]);
  assert.ok(center);

  const mapsByName = Object.fromEntries(
    [
      'northWest',
      'north',
      'northEast',
      'west',
      'center',
      'east',
      'farEast',
    ].map((name) => [name, map(name)]),
  );
  const radiusOne = getGridAdjacentSlots(center, mapsByName, 1);
  assert.equal(radiusOne.length, 8);
  assert.equal(
    radiusOne.some((slot) => slot.offsetX === 0 && slot.offsetY === 0),
    false,
  );
  assert.equal(
    radiusOne.find((slot) => slot.mapName === 'east')?.map?.name,
    'east',
  );
  assert.equal(
    radiusOne.find((slot) => slot.cellX === 1 && slot.cellY === 2)?.map,
    undefined,
  );

  const corner = findMapGridPlacement('northWest', [documentGrid]);
  assert.ok(corner);
  assert.equal(getGridAdjacentSlots(corner, mapsByName, 1).length, 3);
  assert.equal(getGridAdjacentSlots(center, mapsByName, 0).length, 8);
});

test('editable and navigable slot rules preserve distant-map and immediate-blank behavior', () => {
  const assigned = {
    offsetX: 2,
    offsetY: 0,
    cellX: 3,
    cellY: 1,
    mapName: 'farEast',
    map: map('farEast'),
  };
  const immediateBlank = {
    offsetX: 0,
    offsetY: 1,
    cellX: 1,
    cellY: 2,
    mapName: '',
  };
  const distantBlank = { ...immediateBlank, offsetX: 2 };

  assert.equal(isGridSlotEditable(assigned), true);
  assert.equal(isGridSlotNavigable(assigned), true);
  assert.equal(isGridSlotEditable(immediateBlank), false);
  assert.equal(isGridSlotNavigable(immediateBlank), true);
  assert.equal(isGridSlotNavigable(distantBlank), false);
});

test('grid map enumeration returns only loaded assigned maps within the requested radius', () => {
  const documentGrid = grid('world', [['west', 'center', 'east', 'farEast']]);
  const center = findMapGridPlacement('center', [documentGrid]);
  assert.ok(center);
  const mapsByName = {
    center: map('center'),
    east: map('east'),
    farEast: map('farEast'),
  };

  assert.deepEqual(
    getGridMapsWithinRadius(center, mapsByName, 2).map((entry) => [
      entry.map.name,
      entry.offsetX,
      entry.offsetY,
    ]),
    [
      ['east', 1, 0],
      ['farEast', 2, 0],
    ],
  );
});

test('grid layer union is unique, descending, and ignores unloaded cells', () => {
  const documentGrid = grid('world', [['center', 'east', 'missing']]);
  assert.deepEqual(
    getGridLayerSet(documentGrid, {
      center: map('center', 4, 3, [0, 2]),
      east: map('east', 4, 3, [-1, 2]),
    }),
    [2, 0, -1],
  );
});

test('brush resolution crosses positive and negative grid seams with local tile indices', () => {
  const documentGrid = grid('world', [
    ['northWest', 'north', 'northEast'],
    ['west', 'center', 'east'],
    ['southWest', 'south', 'southEast'],
  ]);
  const mapsByName = Object.fromEntries(
    documentGrid.cells.flat().map((name) => [name, map(name)]),
  );
  const center = mapsByName.center;

  assert.deepEqual(
    resolveGridBrushCell(center, 2, 1, [documentGrid], mapsByName),
    {
      map: center,
      tileIndex: 6,
    },
  );
  assert.equal(
    resolveGridBrushCell(center, -1, 0, [documentGrid], mapsByName)?.map.name,
    'west',
  );
  assert.equal(
    resolveGridBrushCell(center, -1, 0, [documentGrid], mapsByName)?.tileIndex,
    3,
  );
  assert.equal(
    resolveGridBrushCell(center, 4, 2, [documentGrid], mapsByName)?.map.name,
    'east',
  );
  assert.equal(
    resolveGridBrushCell(center, 4, 2, [documentGrid], mapsByName)?.tileIndex,
    8,
  );
  assert.equal(
    resolveGridBrushCell(center, -1, -1, [documentGrid], mapsByName)?.map.name,
    'northWest',
  );
  assert.equal(
    resolveGridBrushCell(center, -1, -1, [documentGrid], mapsByName)?.tileIndex,
    11,
  );
});

test('brush resolution rejects off-grid, unloaded, and incompatible neighbor targets', () => {
  const documentGrid = grid('world', [['center', 'east', 'missing']]);
  const center = map('center');
  const undersizedEast = map('east', 3, 3);
  const mapsByName = { center, east: undersizedEast };

  assert.equal(
    resolveGridBrushCell(center, -1, 0, [documentGrid], mapsByName),
    null,
  );
  assert.equal(
    resolveGridBrushCell(center, 8, 0, [documentGrid], mapsByName),
    null,
  );
  assert.equal(
    resolveGridBrushCell(center, 7, 0, [documentGrid], mapsByName),
    null,
  );
  assert.equal(
    resolveGridBrushCell(center, 4, 0, [documentGrid], mapsByName),
    null,
  );
  assert.equal(resolveGridBrushCell(center, 4, 0, [], { center }), null);
});

test('grid assignment and rename return updated cells without mutating their inputs', () => {
  const original = grid('world', [['center', '']]);
  const assigned = assignMapToGridCell([original], 'world', 1, 0, 'east');
  assert.deepEqual(original.cells, [['center', '']]);
  assert.deepEqual(assigned[0].cells, [['center', 'east']]);

  const renamed = renameMapInGrids(assigned, ' east ', ' eastRenamed ');
  assert.deepEqual(assigned[0].cells, [['center', 'east']]);
  assert.deepEqual(renamed[0].cells, [['center', 'eastRenamed']]);
});

test('visible tile range clips to the viewport and honors overscan', () => {
  const base = {
    originX: -64,
    originY: -32,
    canvasWidth: 96,
    canvasHeight: 64,
    mapWidth: 100,
    mapHeight: 100,
    tileWidth: 16,
    tileHeight: 16,
    scale: 1,
  };

  assert.deepEqual(getVisibleTileRange({ ...base, marginTiles: 0 }), {
    minX: 4,
    maxX: 10,
    minY: 2,
    maxY: 6,
  });
  assert.deepEqual(getVisibleTileRange({ ...base, marginTiles: 2 }), {
    minX: 2,
    maxX: 12,
    minY: 0,
    maxY: 8,
  });
});

test('visible tile range returns null off-screen and preserves the legacy invalid-scale fallback', () => {
  const args = {
    originX: 500,
    originY: 500,
    canvasWidth: 100,
    canvasHeight: 100,
    mapWidth: 10,
    mapHeight: 8,
    tileWidth: 16,
    tileHeight: 16,
    scale: 1,
    marginTiles: 0,
  };
  assert.equal(getVisibleTileRange(args), null);
  assert.deepEqual(getVisibleTileRange({ ...args, scale: 0 }), {
    minX: 0,
    maxX: 9,
    minY: 0,
    maxY: 7,
  });
});
