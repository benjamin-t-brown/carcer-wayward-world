import assert from 'node:assert/strict';
import test from 'node:test';

import {
  createBlankMapGrid,
  parseMapGridRecord,
} from '../../src/core/domain/mapGrids/index.js';
import { parseMapRecord } from '../../src/core/domain/maps/index.js';

test('grid creation eagerly creates stable, API-named blank partitions', () => {
  const suffixes = ['Taken!', 'A-1', 'B_2', 'C3', 'D4'];
  const result = createBlankMapGrid({
    name: 'North Reach',
    label: 'North Reach',
    gridWidth: 2,
    gridHeight: 2,
    mapWidth: 30,
    mapHeight: 20,
    layers: [1, 0],
    tilesets: ['', 'terrain'],
    existingMapNames: ['north_reach_taken'],
    nextSuffix: () => suffixes.shift() ?? 'fallback',
  });

  assert.deepEqual(result.grid.cells, [
    ['north_reach_a1', 'north_reach_b2'],
    ['north_reach_c3', 'north_reach_d4'],
  ]);
  assert.equal(result.maps.length, 4);
  assert.deepEqual(
    result.maps.map(({ name }) => name),
    result.grid.cells.flat(),
  );
  for (const map of result.maps) {
    assert.equal(map.width, 30);
    assert.equal(map.height, 20);
    assert.deepEqual(map.layers, [1, 0]);
    assert.equal(map.tiles['0']?.length, 1200);
    assert.equal(map.tiles['1']?.length, 1200);
    assert.ok(map.tiles['0']?.every((value) => value === 0));
    parseMapRecord(map);
  }
  parseMapGridRecord(result.grid);
});

test('generated map API names remain stored and do not depend on coordinates', () => {
  let sequence = 0;
  const result = createBlankMapGrid({
    name: 'Grid',
    gridWidth: 2,
    gridHeight: 1,
    mapWidth: 1,
    mapHeight: 1,
    nextSuffix: () => `random${(sequence += 1)}`,
  });
  const snapshot = structuredClone(result.grid);

  assert.deepEqual(snapshot.cells, [['grid_random1', 'grid_random2']]);
  assert.deepEqual(result.grid, snapshot);
});

test('grid creation rejects invalid dimensions before allocating maps', () => {
  assert.throws(
    () =>
      createBlankMapGrid({
        name: 'bad',
        gridWidth: 0,
        gridHeight: 1,
        mapWidth: 20,
        mapHeight: 20,
      }),
    /gridWidth must be a positive safe integer/,
  );
});
