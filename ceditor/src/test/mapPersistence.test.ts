import assert from 'node:assert/strict';
import test from 'node:test';

import type {
  CarcerMapTemplate,
  MapGridTemplate,
} from '../client/types/assets';
import {
  prepareGridMapCreationCandidates,
  prepareMapRenameCandidates,
  prepareMapsSaveCandidate,
} from '../client/utils/mapPersistence';

function map(name: string): CarcerMapTemplate {
  return {
    name,
    label: name,
    type: 'TOWN',
    width: 2,
    height: 1,
    spriteWidth: 16,
    spriteHeight: 16,
    tilesets: [''],
    layers: [0],
    tiles: { '0': [0, 0, 0, 0] },
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };
}

function grid(cells: string[][]): MapGridTemplate {
  return {
    name: 'world',
    label: 'World',
    gridWidth: cells[0]?.length ?? 1,
    gridHeight: cells.length,
    mapWidth: 2,
    mapHeight: 1,
    cells,
  };
}

test('grid creation produces one coherent maps and map-grids candidate', () => {
  const originalMaps = [map('west')];
  const originalGrids = [grid([['west', '']])];
  const created = map(' east ');

  const candidate = prepareGridMapCreationCandidates(
    originalMaps,
    originalGrids,
    created,
    { gridName: 'world', cellX: 1, cellY: 0 },
  );

  assert.deepEqual(
    candidate.maps.map(({ name }) => name),
    ['west', 'east'],
  );
  assert.equal(candidate.createdMap.name, 'east');
  assert.deepEqual(candidate.mapGrids[0]?.cells, [['west', 'east']]);
  assert.deepEqual(originalGrids[0]?.cells, [['west', '']]);
  assert.deepEqual(
    originalMaps.map(({ name }) => name),
    ['west'],
  );
});

test('map rename updates the map and every grid reference in one candidate', () => {
  const originalMaps = [map('old-name'), map('standalone')];
  const originalGrids = [
    grid([
      [' old-name ', ''],
      ['', 'old-name'],
    ]),
  ];
  const edited = { ...originalMaps[0]!, name: ' new-name ', label: ' New ' };

  const candidate = prepareMapRenameCandidates(
    originalMaps,
    originalGrids,
    'old-name',
    edited,
  );

  assert.ok(candidate);
  assert.equal(candidate.renamed, true);
  assert.equal(candidate.maps[0]?.name, 'new-name');
  assert.equal(candidate.maps[0]?.label, 'New');
  assert.deepEqual(candidate.mapGrids[0]?.cells, [
    ['new-name', ''],
    ['', 'new-name'],
  ]);
  assert.equal(originalMaps[0]?.name, 'old-name');
  assert.deepEqual(originalGrids[0]?.cells, [
    [' old-name ', ''],
    ['', 'old-name'],
  ]);
});

test('save preparation replaces the active canvas map and trims a deep copy', () => {
  const maps = [map('active'), map('other')];
  maps[1]!.label = ' Other ';
  const flushedCanvasMap = {
    ...maps[0]!,
    label: ' Active ',
    tiles: { '0': [0, 0, 1, 7] },
  };

  const candidate = prepareMapsSaveCandidate(maps, flushedCanvasMap);

  assert.deepEqual(candidate[0]?.tiles['0'], [0, 0, 1, 7]);
  assert.equal(candidate[0]?.label, 'Active');
  assert.equal(candidate[1]?.label, 'Other');
  assert.deepEqual(maps[0]?.tiles['0'], [0, 0, 0, 0]);
  assert.equal(maps[1]?.label, ' Other ');
});
