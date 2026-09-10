import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import {
  buildMapGridTopologyIndex,
  MapGridParseError,
  MapGridTopology,
  parseMapGridCollection,
  parseMapGridRecord,
  type MapGridRecord,
} from '../../src/core/domain/mapGrids/index.js';

function gridFixture(overrides: Partial<MapGridRecord> = {}): MapGridRecord {
  return {
    name: 'town-grid',
    label: 'Town Grid',
    gridWidth: 3,
    gridHeight: 2,
    mapWidth: 30,
    mapHeight: 20,
    cells: [
      ['northwest', '', ' northeast '],
      ['southwest', 'center', 'center'],
    ],
    futureGridField: { version: 2 },
    ...overrides,
  };
}

test('parser preserves unknown data, whitespace, and caller ownership', () => {
  const input = gridFixture();
  const parsed = parseMapGridRecord(input);

  input.cells[0]![0] = 'changed';
  (input.futureGridField as { version: number }).version = 3;

  assert.equal(parsed.cells[0]![0], 'northwest');
  assert.equal(parsed.cells[0]![2], ' northeast ');
  assert.deepEqual(parsed.futureGridField, { version: 2 });
});

test('parser reports exact paths for malformed dimensions and cells', () => {
  assert.throws(
    () => parseMapGridRecord(gridFixture({ gridWidth: 0 }), 'mapGrids[4]'),
    (error: unknown) => {
      assert.ok(error instanceof MapGridParseError);
      assert.equal(error.path, 'mapGrids[4].gridWidth');
      assert.equal(error.detail, 'expected a positive safe integer');
      return true;
    },
  );

  const wrongHeight = gridFixture({ gridHeight: 3 });
  assert.throws(
    () => parseMapGridRecord(wrongHeight, 'mapGrids[1]'),
    (error: unknown) => {
      assert.ok(error instanceof MapGridParseError);
      assert.equal(error.path, 'mapGrids[1].cells');
      assert.match(error.detail, /expected 3 rows, received 2/);
      return true;
    },
  );

  const wrongWidth = gridFixture();
  wrongWidth.cells[1]!.pop();
  assert.throws(
    () => parseMapGridRecord(wrongWidth, 'mapGrids[2]'),
    (error: unknown) => {
      assert.ok(error instanceof MapGridParseError);
      assert.equal(error.path, 'mapGrids[2].cells[1]');
      return true;
    },
  );

  const wrongCell = gridFixture() as unknown as {
    cells: unknown[][];
  };
  wrongCell.cells[0]![2] = null;
  assert.throws(
    () => parseMapGridRecord(wrongCell, 'mapGrids[3]'),
    (error: unknown) => {
      assert.ok(error instanceof MapGridParseError);
      assert.equal(error.path, 'mapGrids[3].cells[0][2]');
      return true;
    },
  );
});

test('collection parser includes the failing record index in its path', () => {
  assert.throws(
    () =>
      parseMapGridCollection([gridFixture(), { ...gridFixture(), cells: 3 }]),
    (error: unknown) => {
      assert.ok(error instanceof MapGridParseError);
      assert.equal(error.path, 'mapGrids[1].cells');
      return true;
    },
  );
});

test('topology enumerates assigned blocks with tile-space origins', () => {
  const topology = MapGridTopology.from(gridFixture());

  assert.deepEqual(topology.blocks(), [
    {
      gridName: 'town-grid',
      mapName: 'northwest',
      cellX: 0,
      cellY: 0,
      originTileX: 0,
      originTileY: 0,
    },
    {
      gridName: 'town-grid',
      mapName: 'northeast',
      cellX: 2,
      cellY: 0,
      originTileX: 60,
      originTileY: 0,
    },
    {
      gridName: 'town-grid',
      mapName: 'southwest',
      cellX: 0,
      cellY: 1,
      originTileX: 0,
      originTileY: 20,
    },
    {
      gridName: 'town-grid',
      mapName: 'center',
      cellX: 1,
      cellY: 1,
      originTileX: 30,
      originTileY: 20,
    },
    {
      gridName: 'town-grid',
      mapName: 'center',
      cellX: 2,
      cellY: 1,
      originTileX: 60,
      originTileY: 20,
    },
  ]);
  assert.deepEqual(topology.originAt(2, 1), { tileX: 60, tileY: 20 });
  assert.equal(topology.originAt(3, 1), undefined);
  assert.equal(topology.blockAt(1, 0), undefined);
  assert.equal(topology.mapNameAt(2, 0), 'northeast');
  assert.equal(topology.mapNameAt(1, 0), undefined);
});

test('duplicate map references remain visible with a deterministic first placement', () => {
  const topology = MapGridTopology.from(gridFixture());

  assert.deepEqual(
    topology.placementsOf(' center ').map(({ cellX, cellY }) => ({
      cellX,
      cellY,
    })),
    [
      { cellX: 1, cellY: 1 },
      { cellX: 2, cellY: 1 },
    ],
  );
  assert.deepEqual(topology.placementOf('center'), topology.blocks()[3]);
  assert.deepEqual(topology.placementsOf(''), []);
});

test('neighbor slots include empty cells and omit out-of-bounds cells', () => {
  const topology = MapGridTopology.from(gridFixture());
  const slots = topology.neighborSlots({ cellX: 0, cellY: 0 });

  assert.deepEqual(slots, [
    {
      gridName: 'town-grid',
      cellX: 1,
      cellY: 0,
      offsetX: 1,
      offsetY: 0,
      originTileX: 30,
      originTileY: 0,
    },
    {
      gridName: 'town-grid',
      cellX: 0,
      cellY: 1,
      offsetX: 0,
      offsetY: 1,
      originTileX: 0,
      originTileY: 20,
      mapName: 'southwest',
    },
    {
      gridName: 'town-grid',
      cellX: 1,
      cellY: 1,
      offsetX: 1,
      offsetY: 1,
      originTileX: 30,
      originTileY: 20,
      mapName: 'center',
    },
  ]);
  assert.deepEqual(topology.neighborSlots({ cellX: 0, cellY: 0 }, 0), []);
  assert.deepEqual(topology.neighborSlots({ cellX: -1, cellY: 0 }), []);
});

test('cross-grid index preserves source-grid and row-major duplicate order', () => {
  const second = gridFixture({
    name: 'second-grid',
    gridWidth: 1,
    gridHeight: 1,
    cells: [['center']],
  });
  const index = buildMapGridTopologyIndex([gridFixture(), second]);

  assert.deepEqual(
    index.placementsOf('center').map(({ gridName, cellX }) => ({
      gridName,
      cellX,
    })),
    [
      { gridName: 'town-grid', cellX: 1 },
      { gridName: 'town-grid', cellX: 2 },
      { gridName: 'second-grid', cellX: 0 },
    ],
  );
  assert.equal(index.placementOf('center')?.gridName, 'town-grid');
  assert.equal(index.placementOf('missing'), undefined);
});

test('topology snapshot preserves exact source values without exposing internals', () => {
  const topology = MapGridTopology.from(gridFixture());
  const snapshot = topology.snapshot();
  snapshot.cells[0]![0] = 'changed';

  assert.equal(topology.mapNameAt(0, 0), 'northwest');
  assert.equal(topology.snapshot().cells[0]![0], 'northwest');
  assert.equal(topology.snapshot().cells[0]![2], ' northeast ');
  assert.deepEqual(topology.snapshot().futureGridField, { version: 2 });
});

test('the current game map-grid file satisfies the topology contract', async () => {
  const source = await readFile(
    new URL('../../../src/assets/db/map-grids.json', import.meta.url),
    'utf8',
  );
  const records = parseMapGridCollection(JSON.parse(source));
  const index = buildMapGridTopologyIndex(records);

  assert.equal(records.length, 2);
  assert.equal(index.grids[0]?.name, 'Alinea');
  assert.equal(index.grids[0]?.blocks().length, 18);
  assert.equal(index.grids[1]?.blocks().length, 0);
  assert.deepEqual(index.placementOf('alinea_outsideAlinea1'), {
    gridName: 'Alinea',
    mapName: 'alinea_outsideAlinea1',
    cellX: 1,
    cellY: 1,
    originTileX: 30,
    originTileY: 30,
  });
});
