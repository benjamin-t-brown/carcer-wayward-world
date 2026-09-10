import assert from 'node:assert/strict';
import test from 'node:test';

import {
  buildMapDocumentIndex,
  MapDocumentIndexCache,
  type MapDocumentIndexAssets,
} from '../client/tile-editor/mapDocumentIndex';
import { buildMapRenderPlan } from '../client/tile-editor/mapRenderPlan';
import { findAdjacentGridSlotAtCanvasPoint } from '../client/tile-editor/gridMapNavigation';
import type {
  CarcerMapTemplate,
  CharacterTemplate,
  GameEvent,
  ItemTemplate,
  MapGridTemplate,
  TilesetTemplate,
} from '../client/types/assets';
import type { MapGridPlacement } from '../client/utils/mapGridIndex';

function map(name: string, width = 30, height = 30): CarcerMapTemplate {
  return {
    name,
    label: name,
    type: 'TOWN',
    width,
    height,
    spriteWidth: 16,
    spriteHeight: 16,
    tilesets: [''],
    layers: [0],
    tiles: { '0': [] },
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };
}

function sparseGrid(
  size: number,
  anchorX: number,
  anchorY: number,
  maps: CarcerMapTemplate[],
): MapGridTemplate {
  const cells = new Array<string[]>(size);
  let nextMap = 0;
  for (let dy = -1; dy <= 1; dy++) {
    const row = new Array<string>(size);
    for (let dx = -1; dx <= 1; dx++) {
      row[anchorX + dx] = maps[nextMap++]!.name;
    }
    cells[anchorY + dy] = row;
  }
  return {
    name: `grid-${size}`,
    label: `Grid ${size}`,
    gridWidth: size,
    gridHeight: size,
    mapWidth: 30,
    mapHeight: 30,
    cells,
  };
}

function placement(
  grid: MapGridTemplate,
  cellX: number,
  cellY: number,
): MapGridPlacement {
  return { grid, cellX, cellY };
}

function visibleTileCount(plan: ReturnType<typeof buildMapRenderPlan>): number {
  return plan.partitions.reduce((count, partition) => {
    const range = partition.visibleTiles;
    return range
      ? count + (range.maxX - range.minX + 1) * (range.maxY - range.minY + 1)
      : count;
  }, 0);
}

test('huge-grid planning visits only viewport-intersecting cells', () => {
  const maps = Array.from({ length: 9 }, (_, i) => map(`partition-${i}`));
  const anchorX = 50_000;
  const anchorY = 50_000;
  const hugeGrid = sparseGrid(100_000, anchorX, anchorY, maps);
  const mapsByName = new Map(maps.map((entry) => [entry.name, entry]));
  const focusedMap = maps[4]!;

  const plan = buildMapRenderPlan({
    focusedMap,
    canvasWidth: 960,
    canvasHeight: 960,
    focusOriginX: 240,
    focusOriginY: 240,
    scale: 1,
    mapsByName,
    placement: placement(hugeGrid, anchorX, anchorY),
    partitionOverscan: 0,
    tileOverscan: 0,
  });

  assert.equal(plan.visitedCellCount, 9);
  assert.equal(plan.cells.length, plan.visitedCellCount);
  assert.equal(plan.partitions.length, 9);
  assert.equal(plan.partitions.at(-1)?.map, focusedMap);
  assert.equal(plan.partitions.at(-1)?.focused, true);
  const partitionColumns = new Set(
    plan.partitions.map((partition) => partition.cellX),
  ).size;
  const partitionRows = new Set(
    plan.partitions.map((partition) => partition.cellY),
  ).size;
  const maxVisibleColumns =
    Math.ceil(960 / focusedMap.spriteWidth) + partitionColumns;
  const maxVisibleRows =
    Math.ceil(960 / focusedMap.spriteHeight) + partitionRows;
  assert.ok(visibleTileCount(plan) <= maxVisibleColumns * maxVisibleRows);
  assert.ok(plan.visitedCellCount < hugeGrid.gridWidth * hugeGrid.gridHeight);
});

test('changing total grid size does not change fixed-viewport work', () => {
  const maps = Array.from({ length: 9 }, (_, i) => map(`fixed-${i}`));
  const mapsByName = new Map(maps.map((entry) => [entry.name, entry]));

  const plannedWork = (size: number) => {
    const anchor = Math.floor(size / 2);
    const grid = sparseGrid(size, anchor, anchor, maps);
    const plan = buildMapRenderPlan({
      focusedMap: maps[4]!,
      canvasWidth: 960,
      canvasHeight: 960,
      focusOriginX: 240,
      focusOriginY: 240,
      scale: 1,
      mapsByName,
      placement: placement(grid, anchor, anchor),
      partitionOverscan: 0,
      tileOverscan: 0,
    });
    return {
      visitedCells: plan.visitedCellCount,
      partitions: plan.partitions.length,
      visibleTiles: visibleTileCount(plan),
    };
  };

  assert.deepEqual(plannedWork(9), plannedWork(100_000));
});

test('offscreen grid maps are omitted without reading or materializing tiles', () => {
  const focused = map('visible-focus');
  const offscreen = map('far-away');
  Object.defineProperty(offscreen, 'tiles', {
    configurable: true,
    get(): never {
      throw new Error('offscreen tiles must not be read while planning');
    },
  });
  const size = 10_000;
  const cells = new Array<string[]>(size);
  const focusRow = new Array<string>(size);
  focusRow[5_000] = focused.name;
  focusRow[9_999] = offscreen.name;
  cells[5_000] = focusRow;
  const grid: MapGridTemplate = {
    name: 'offscreen-grid',
    label: 'Offscreen grid',
    gridWidth: size,
    gridHeight: size,
    mapWidth: 30,
    mapHeight: 30,
    cells,
  };

  const plan = buildMapRenderPlan({
    focusedMap: focused,
    canvasWidth: 480,
    canvasHeight: 480,
    focusOriginX: 0,
    focusOriginY: 0,
    scale: 1,
    mapsByName: new Map([
      [focused.name, focused],
      [offscreen.name, offscreen],
    ]),
    placement: placement(grid, 5_000, 5_000),
    partitionOverscan: 0,
  });

  assert.deepEqual(
    plan.partitions.map(({ mapName }) => mapName),
    [focused.name],
  );
  assert.equal(plan.visitedCellCount, 1);
});

test('standalone maps use the same single-partition plan', () => {
  const standalone = map('standalone-plan', 1_000, 1_000);
  const onscreen = buildMapRenderPlan({
    focusedMap: standalone,
    canvasWidth: 320,
    canvasHeight: 240,
    focusOriginX: -8_000,
    focusOriginY: -8_000,
    scale: 1,
    mapsByName: new Map([[standalone.name, standalone]]),
    tileOverscan: 0,
  });

  assert.equal(onscreen.grid, undefined);
  assert.equal(onscreen.visitedCellCount, 1);
  assert.equal(onscreen.partitions.length, 1);
  assert.equal(onscreen.cells.length, 1);
  assert.equal(onscreen.cells[0]?.navigable, false);
  assert.equal(onscreen.partitions[0]?.focused, true);
  assert.ok(visibleTileCount(onscreen) <= 21 * 16);

  const offscreen = buildMapRenderPlan({
    focusedMap: standalone,
    canvasWidth: 320,
    canvasHeight: 240,
    focusOriginX: 1_000,
    focusOriginY: 1_000,
    scale: 1,
    mapsByName: new Map([[standalone.name, standalone]]),
  });
  assert.equal(offscreen.partitions.length, 1);
  assert.equal(offscreen.partitions[0]?.visibleTiles, null);
});

test('bounded grid cells preserve loaded-map and immediate-blank navigation', () => {
  const focused = map('navigation-focus');
  const farLoaded = map('navigation-far');
  const grid: MapGridTemplate = {
    name: 'navigation-grid',
    label: 'Navigation grid',
    gridWidth: 5,
    gridHeight: 1,
    mapWidth: 30,
    mapHeight: 30,
    cells: [['missing-map', '', focused.name, '', farLoaded.name]],
  };
  const plan = buildMapRenderPlan({
    focusedMap: focused,
    canvasWidth: 2_400,
    canvasHeight: 480,
    focusOriginX: 960,
    focusOriginY: 0,
    scale: 1,
    mapsByName: new Map([
      [focused.name, focused],
      [farLoaded.name, farLoaded],
    ]),
    placement: placement(grid, 2, 0),
    partitionOverscan: 0,
  });

  assert.deepEqual(
    plan.cells.map(({ offsetX, mapName, navigable }) => ({
      offsetX,
      mapName,
      navigable,
    })),
    [
      { offsetX: -2, mapName: 'missing-map', navigable: false },
      { offsetX: -1, mapName: '', navigable: true },
      { offsetX: 0, mapName: focused.name, navigable: false },
      { offsetX: 1, mapName: '', navigable: true },
      { offsetX: 2, mapName: farLoaded.name, navigable: true },
    ],
  );
  assert.deepEqual(
    plan.partitions.map(({ mapName }) => mapName),
    [farLoaded.name, focused.name],
  );
});

test('navigation hit testing reaches a distant loaded partition after panning', () => {
  const focused = map('hit-focus');
  const distant = map('hit-distant');
  const grid: MapGridTemplate = {
    name: 'hit-grid',
    label: 'Hit grid',
    gridWidth: 4,
    gridHeight: 1,
    mapWidth: 30,
    mapHeight: 30,
    cells: [[focused.name, '', '', distant.name]],
  };
  const canvas = { width: 480, height: 480 } as HTMLCanvasElement;

  const hit = findAdjacentGridSlotAtCanvasPoint({
    canvasX: 240,
    canvasY: 240,
    canvas,
    map: focused,
    mapGrids: [grid],
    maps: [focused, distant],
    mapsByName: new Map([
      [focused.name, focused],
      [distant.name, distant],
    ]),
    placement: placement(grid, 0, 0),
    translateX: -1_440,
    translateY: 0,
    scale: 1,
  });

  assert.equal(hit?.slot.offsetX, 3);
  assert.equal(hit?.slot.map, distant);
});

test('invalid viewport produces no grid work', () => {
  const focused = map('invalid-viewport');
  const grid = sparseGrid(3, 1, 1, [
    map('invalid-0'),
    map('invalid-1'),
    map('invalid-2'),
    map('invalid-3'),
    focused,
    map('invalid-5'),
    map('invalid-6'),
    map('invalid-7'),
    map('invalid-8'),
  ]);
  const plan = buildMapRenderPlan({
    focusedMap: focused,
    canvasWidth: 0,
    canvasHeight: 480,
    focusOriginX: 0,
    focusOriginY: 0,
    scale: 1,
    mapsByName: new Map([[focused.name, focused]]),
    placement: placement(grid, 1, 1),
  });

  assert.equal(plan.visitedCellCount, 0);
  assert.deepEqual(plan.cells, []);
  assert.deepEqual(plan.partitions, []);
});

test('document index provides direct asset and tile-definition lookups', () => {
  const firstMap = map('indexed-map');
  const duplicateMap = map('indexed-map');
  duplicateMap.label = 'duplicate';
  const tileset = {
    name: 'indexed-tileset',
    spriteBase: 'tiles',
    imageWidth: 32,
    imageHeight: 16,
    tileWidth: 16,
    tileHeight: 16,
    tiles: [
      { id: 0, description: 'first' },
      { id: 1, description: 'second' },
    ],
  } satisfies TilesetTemplate;
  const character = { name: 'indexed-character' } as CharacterTemplate;
  const item = { name: 'indexed-item' } as ItemTemplate;
  const event = { id: 'indexed-event' } as GameEvent;
  const documentGrid: MapGridTemplate = {
    name: 'indexed-grid',
    label: 'Indexed grid',
    gridWidth: 2,
    gridHeight: 1,
    mapWidth: 30,
    mapHeight: 30,
    cells: [[firstMap.name, firstMap.name]],
  };
  const assets: MapDocumentIndexAssets = {
    maps: [firstMap, duplicateMap],
    mapGrids: [documentGrid],
    tilesets: [tileset],
    characters: [character],
    items: [item],
    gameEvents: [event],
  };

  const index = buildMapDocumentIndex('revision-1', assets);
  assert.equal(index.mapsByName.get(firstMap.name), firstMap);
  assert.equal(index.gridsByName.get(documentGrid.name), documentGrid);
  assert.equal(index.tilesetsByName.get(tileset.name), tileset);
  assert.equal(
    index.tileDefinitionsByTilesetName.get(tileset.name)?.get(1),
    tileset.tiles[1],
  );
  assert.equal(index.charactersByName.get(character.name), character);
  assert.equal(index.itemsByName.get(item.name), item);
  assert.equal(index.eventsById.get(event.id), event);
  assert.deepEqual(
    index.placementsByMapName
      .get(firstMap.name)
      ?.map(({ cellX, cellY }) => [cellX, cellY]),
    [
      [0, 0],
      [1, 0],
    ],
  );
});

test('document index cache keys by revision and collection identity', () => {
  const assets: MapDocumentIndexAssets = {
    maps: [map('cache-map')],
    mapGrids: [],
    tilesets: [],
    characters: [],
    items: [],
    gameEvents: [],
  };
  const cache = new MapDocumentIndexCache();

  const first = cache.get('r1', assets);
  assert.equal(cache.get('r1', assets), first);
  assert.notEqual(cache.get('r2', assets), first);

  const changedMaps = { ...assets, maps: [...assets.maps] };
  const changed = cache.get('r2', changedMaps);
  assert.notEqual(changed, first);
  assert.equal(cache.get('r2', changedMaps), changed);

  cache.clear();
  assert.notEqual(cache.get('r2', changedMaps), changed);
});
