import assert from 'node:assert/strict';
import test from 'node:test';

import type { MapGridRecord } from '../../src/core/domain/mapGrids/index.js';
import {
  addMapLayer,
  cloneMapInCollection,
  cloneMapRecord,
  createMapInCollection,
  deleteMapAcrossDatabase,
  deleteMapLayer,
  MapLifecycleError,
  previewMapReferences,
  renameMapAcrossDatabase,
  renameMapRecord,
  resizeMapRecord,
  updateMapMetadata,
  type MapRecord,
} from '../../src/core/domain/maps/index.js';

function mapFixture(
  name = 'old_map',
  overrides: Partial<MapRecord> = {},
): MapRecord {
  return {
    name,
    label: 'Old map',
    type: 'TOWN',
    width: 3,
    height: 2,
    spriteWidth: 28,
    spriteHeight: 32,
    tilesets: ['', 'terrain'],
    layers: [1, 0],
    tiles: {
      '1': [1, 10, 1, 11, 1, 12, 1, 13, 1, 14, 1, 15],
      '0': [1, 20, 1, 21, 1, 22, 1, 23, 1, 24, 1, 25],
    },
    characters: [
      { l: 0, i: 0, name: 'origin', futurePlacement: 'keep' },
      { l: 0, i: 2, name: 'cropped-x' },
      { l: 0, i: 3, name: 'translated' },
      { l: 0, i: 5, name: 'cropped-y' },
    ],
    items: [{ l: 1, i: 4, name: 'item', quantity: 2 }],
    markers: [{ name: 'implicit-origin' }],
    eventTriggers: [{ l: 1, i: 1, eventId: 'event' }],
    travelTriggers: [],
    tileOverrides: [{ l: 0, i: 3, overrides: { futureOverride: 1 } }],
    lightSources: [{ l: 0, i: 4, radius: 3 }],
    futureMapField: { keep: true },
    ...overrides,
  };
}

function gridFixture(
  name = 'world_grid',
  cells: string[][] = [['old_map', 'other_map']],
): MapGridRecord {
  return {
    name,
    label: 'World',
    gridWidth: cells[0]!.length,
    gridHeight: cells.length,
    mapWidth: 3,
    mapHeight: 2,
    cells,
    futureGridField: { keep: true },
  };
}

test('clone and metadata operations retain unknown fields and omitted fields', () => {
  const source = mapFixture();
  delete source.label;
  delete source.spriteHeight;

  const clone = cloneMapRecord(source, { name: ' copy_map ' });
  const labeledClone = cloneMapRecord(source, {
    name: 'labeled_copy',
    label: 'A copy',
  });
  const renamed = renameMapRecord(source, 'renamed');
  const metadata = updateMapMetadata(source, {
    label: 'New label',
    type: null,
    spriteWidth: 16,
    spriteHeight: null,
  });

  assert.equal(clone.name, 'copy_map');
  assert.equal('label' in clone, false);
  assert.equal('spriteHeight' in clone, false);
  assert.deepEqual(clone.futureMapField, { keep: true });
  assert.equal(labeledClone.label, 'A copy');
  assert.equal(renamed.name, 'renamed');
  assert.equal(metadata.label, 'New label');
  assert.equal('type' in metadata, false);
  assert.equal(metadata.spriteWidth, 16);
  assert.equal('spriteHeight' in metadata, false);

  clone.characters![0]!.name = 'changed';
  assert.equal(source.characters![0]!.name, 'origin');
});

test('collection create and clone enforce unique trimmed map names', () => {
  const source = mapFixture();
  const created = createMapInCollection([source], {
    name: 'new_map',
    width: 2,
    height: 1,
  });
  const cloned = cloneMapInCollection(created, 'old_map', {
    name: 'clone_map',
  });

  assert.deepEqual(
    cloned.map((map) => map.name),
    ['old_map', 'new_map', 'clone_map'],
  );
  assert.deepEqual(cloned[2]!.futureMapField, { keep: true });
  assert.throws(
    () =>
      createMapInCollection([source], {
        name: ' old_map ',
        width: 1,
        height: 1,
      }),
    MapLifecycleError,
  );
  assert.throws(
    () => cloneMapInCollection([source], 'missing', { name: 'clone' }),
    /not found/,
  );
  assert.throws(
    () =>
      createMapInCollection(
        [source],
        { name: 'world_grid', width: 1, height: 1 },
        [gridFixture()],
      ),
    /conflicts with an existing map grid/,
  );
});

test('resize preserves top-left graphics and translates or drops sparse placements', () => {
  const source = mapFixture();
  const { map, droppedPlacements } = resizeMapRecord(source, {
    width: 2,
    height: 2,
  });

  assert.equal(map.width, 2);
  assert.equal(map.height, 2);
  assert.deepEqual(map.tiles['1'], [1, 10, 1, 11, 1, 13, 1, 14]);
  assert.deepEqual(map.tiles['0'], [1, 20, 1, 21, 1, 23, 1, 24]);
  assert.deepEqual(map.characters, [
    { l: 0, i: 0, name: 'origin', futurePlacement: 'keep' },
    { l: 0, i: 2, name: 'translated' },
  ]);
  assert.deepEqual(map.markers, [{ name: 'implicit-origin' }]);
  assert.deepEqual(map.items, [{ l: 1, i: 3, name: 'item', quantity: 2 }]);
  assert.deepEqual(map.tileOverrides, [
    { l: 0, i: 2, overrides: { futureOverride: 1 } },
  ]);
  assert.deepEqual(map.lightSources, [{ l: 0, i: 3, radius: 3 }]);
  assert.deepEqual(droppedPlacements, {
    characters: 2,
    items: 0,
    markers: 0,
    eventTriggers: 0,
    travelTriggers: 0,
    tileOverrides: 0,
    lightSources: 0,
  });
  assert.deepEqual(map.futureMapField, { keep: true });
  assert.equal(source.width, 3);
  assert.equal(source.characters![2]!.i, 3);
});

test('resize grows dense layers with an explicit fill graphic', () => {
  const source = mapFixture('small', {
    width: 1,
    height: 1,
    layers: [0],
    tiles: { '0': [1, 8] },
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  });
  const result = resizeMapRecord(source, {
    width: 2,
    height: 2,
    fill: { tilesetIndex: 3, tileIndex: 7 },
  });

  assert.deepEqual(result.map.tiles['0'], [1, 8, 3, 7, 3, 7, 3, 7]);
  assert.throws(
    () => resizeMapRecord(source, { width: 0, height: 1 }),
    /width must be positive/,
  );
  assert.throws(
    () =>
      resizeMapRecord(source, {
        width: 2,
        height: 2,
        fill: { tilesetIndex: 0.5, tileIndex: 0 },
      }),
    /safe integer/,
  );
});

test('layer add preserves order and layer delete removes matching placements', () => {
  const source = mapFixture();
  const added = addMapLayer(source, -1);
  assert.deepEqual(added.layers, [1, 0, -1]);
  assert.deepEqual(added.tiles['-1'], new Array(12).fill(0));
  assert.equal('-1' in source.tiles, false);

  const deleted = deleteMapLayer(source, 0);
  assert.deepEqual(deleted.map.layers, [1]);
  assert.equal('0' in deleted.map.tiles, false);
  assert.deepEqual(deleted.map.characters, []);
  assert.deepEqual(deleted.map.markers, []);
  assert.deepEqual(deleted.map.items, [
    { l: 1, i: 4, name: 'item', quantity: 2 },
  ]);
  assert.equal(deleted.droppedPlacements.characters, 4);
  assert.equal(deleted.droppedPlacements.markers, 1);
  assert.equal(deleted.droppedPlacements.tileOverrides, 1);
  assert.equal(deleted.droppedPlacements.lightSources, 1);

  assert.throws(() => addMapLayer(source, 0), /already exists/);
  assert.throws(() => deleteMapLayer(source, 9), /does not exist/);
  assert.throws(
    () =>
      deleteMapLayer(
        mapFixture('single', {
          layers: [0],
          tiles: { '0': source.tiles['0']! },
        }),
        0,
      ),
    /final map layer/,
  );
});

test('reference preview reports grid cells and travel triggers without mutation', () => {
  const maps = [
    mapFixture(),
    mapFixture('other_map', {
      travelTriggers: [
        {
          l: 0,
          i: 1,
          destinationMapName: ' old_map ',
          destinationX: 4,
          futureTrigger: 'keep',
        },
      ],
    }),
  ];
  const mapGrids = [gridFixture()];
  const preview = previewMapReferences({ maps, mapGrids }, ' old_map ');

  assert.equal(preview.mapIndex, 0);
  assert.equal(preview.travelDestinationResolvesToGrid, false);
  assert.deepEqual(preview.gridCells, [
    { gridIndex: 0, gridName: 'world_grid', cellX: 0, cellY: 0 },
  ]);
  assert.deepEqual(preview.travelTriggers, [
    { mapIndex: 1, mapName: 'other_map', triggerIndex: 0 },
  ]);
  assert.equal(mapGrids[0]!.cells[0]![0], 'old_map');
  assert.equal(maps[1]!.travelTriggers![0]!.destinationMapName, ' old_map ');
});

test('cross-database rename updates grids and travel links losslessly', () => {
  const maps = [
    mapFixture(),
    mapFixture('other_map', {
      travelTriggers: [
        {
          destinationMapName: 'old_map',
          destinationMarkerName: 'door',
          futureTrigger: 'keep',
        },
      ],
    }),
  ];
  const mapGrids = [gridFixture()];
  const result = renameMapAcrossDatabase(
    { maps, mapGrids },
    'old_map',
    'new_map',
  );

  assert.equal(result.maps[0]!.name, 'new_map');
  assert.equal(result.mapGrids[0]!.cells[0]![0], 'new_map');
  assert.deepEqual(result.mapGrids[0]!.futureGridField, { keep: true });
  assert.deepEqual(result.maps[1]!.travelTriggers, [
    {
      destinationMapName: 'new_map',
      destinationMarkerName: 'door',
      futureTrigger: 'keep',
    },
  ]);
  assert.deepEqual(result.maps[0]!.futureMapField, { keep: true });
  assert.equal(maps[0]!.name, 'old_map');
  assert.equal(mapGrids[0]!.cells[0]![0], 'old_map');

  assert.throws(
    () => renameMapAcrossDatabase({ maps, mapGrids }, 'old_map', 'other_map'),
    /already exists/,
  );
  assert.throws(
    () => renameMapAcrossDatabase({ maps, mapGrids }, 'old_map', 'world_grid'),
    /conflicts with an existing map grid/,
  );
});

test('cross-database delete blanks grid cells and clears only destination name', () => {
  const maps = [
    mapFixture(),
    mapFixture('other_map', {
      travelTriggers: [
        {
          destinationMapName: 'old_map',
          destinationMarkerName: 'door',
          destinationX: 4,
          futureTrigger: 'keep',
        },
      ],
    }),
  ];
  const mapGrids = [gridFixture()];
  const result = deleteMapAcrossDatabase({ maps, mapGrids }, 'old_map');

  assert.deepEqual(
    result.maps.map((map) => map.name),
    ['other_map'],
  );
  assert.equal(result.mapGrids[0]!.cells[0]![0], '');
  assert.deepEqual(result.maps[0]!.travelTriggers, [
    {
      destinationMarkerName: 'door',
      destinationX: 4,
      futureTrigger: 'keep',
    },
  ]);
  assert.equal(maps.length, 2);
});

test('same-named grid keeps travel destinations stable during map lifecycle', () => {
  const maps = [
    mapFixture(),
    mapFixture('other_map', {
      travelTriggers: [{ destinationMapName: 'old_map', destinationX: 4 }],
    }),
  ];
  const mapGrids = [gridFixture('old_map')];

  const renamed = renameMapAcrossDatabase(
    { maps, mapGrids },
    'old_map',
    'new_map',
  );
  assert.equal(renamed.preview.travelDestinationResolvesToGrid, true);
  assert.equal(
    renamed.maps[1]!.travelTriggers![0]!.destinationMapName,
    'old_map',
  );
  assert.equal(renamed.mapGrids[0]!.cells[0]![0], 'new_map');

  const deleted = deleteMapAcrossDatabase({ maps, mapGrids }, 'old_map');
  assert.equal(
    deleted.maps[0]!.travelTriggers![0]!.destinationMapName,
    'old_map',
  );
});

test('ambiguous source names are rejected instead of partially updating data', () => {
  const maps = [mapFixture(), mapFixture(' old_map ')];
  assert.throws(
    () => previewMapReferences({ maps, mapGrids: [] }, 'old_map'),
    /ambiguous/,
  );
});
