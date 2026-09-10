import assert from 'node:assert/strict';
import test from 'node:test';

import {
  createUniqueGridName,
  estimateMapGridCost,
  matchesMapGridSearch,
} from '../../src/apps/map-grids/editorModel.js';

test('grid cost exposes eager content size without depending on canvas size', () => {
  assert.deepEqual(estimateMapGridCost(6, 3, 30, 20, 2), {
    partitions: 18,
    tiles: 10_800,
    denseValues: 43_200,
  });
});

test('grid search covers API and display names', () => {
  const grid = {
    name: 'alinea_world',
    label: 'Alinea Outside',
    gridWidth: 1,
    gridHeight: 1,
    mapWidth: 30,
    mapHeight: 30,
    cells: [['map']],
  };
  assert.equal(matchesMapGridSearch(grid, 'WORLD'), true);
  assert.equal(matchesMapGridSearch(grid, 'outside'), true);
  assert.equal(matchesMapGridSearch(grid, 'missing'), false);
});

test('grid names are made unique deterministically', () => {
  assert.equal(
    createUniqueGridName('grid', ['grid', 'grid_2', 'grid_3']),
    'grid_4',
  );
});
