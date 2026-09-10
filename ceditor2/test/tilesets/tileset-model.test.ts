import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import {
  TilesetParseError,
  cloneTilesetRecord,
  createDefaultTileset,
  parseTilesetCollection,
  parseTilesetRecord,
  reconcileTilesToDimensions,
  stepSoundNumber,
} from '../../src/core/domain/tilesets/index.js';

test('parses the live tileset database without normalizing stored values', async () => {
  const source = JSON.parse(
    await readFile('../src/assets/db/tilesets.json', 'utf8'),
  ) as unknown;
  const parsed = parseTilesetCollection(source);

  assert.equal(parsed.length, 4);
  assert.equal(parsed[0]?.name, 'terrain_borders');
  assert.equal(parsed[0]?.tiles?.length, 256);
  assert.equal(parsed[0]?.tiles?.[0]?.stepSound, '1');
  assert.equal(parsed[0]?.tiles?.[13]?.stepSound, 0);
});

test('detaches records while preserving unknown nested fields and nullable sound', () => {
  const source = {
    name: 'future_tiles',
    spriteBase: 'future_picture',
    futureTopLevel: { enabled: true },
    tiles: [
      {
        id: 7,
        stepSound: null,
        futureTile: ['kept'],
        tileTerrainBorderMeta: {
          nw: 'FUTURE_TERRAIN',
          ne: 'NONE',
          sw: 'GRASS',
          se: 'WATER',
          futureCornerOption: 2,
        },
      },
    ],
  };

  const parsed = parseTilesetRecord(source);
  source.futureTopLevel.enabled = false;
  source.tiles[0]!.futureTile[0] = 'changed';

  assert.deepEqual(parsed.futureTopLevel, { enabled: true });
  assert.deepEqual(parsed.tiles?.[0]?.futureTile, ['kept']);
  assert.equal(parsed.tiles?.[0]?.tileTerrainBorderMeta?.futureCornerOption, 2);
  assert.equal(parsed.tiles?.[0]?.stepSound, null);
});

test('reports precise invalid field paths and duplicate loader names', () => {
  assert.throws(
    () =>
      parseTilesetRecord({
        name: 'bad',
        tiles: [{ id: 0, isDoor: 'yes' }],
      }),
    (error: unknown) => {
      assert.ok(error instanceof TilesetParseError);
      assert.equal(error.path, 'tileset.tiles[0].isDoor');
      return true;
    },
  );
  assert.throws(
    () =>
      parseTilesetCollection([
        createDefaultTileset('same'),
        createDefaultTileset('same'),
      ]),
    /tilesets\[1\]\.name: duplicate tileset "same"/,
  );
});

test('clones deeply with a deterministic collision-free name', () => {
  const source = parseTilesetRecord({
    ...createDefaultTileset('terrain'),
    extension: { version: 1 },
  });
  const clone = cloneTilesetRecord(source, [
    'terrain',
    'terrain_copy',
    'terrain_copy2',
  ]);
  clone.tiles![0]!.description = 'clone';
  (clone.extension as { version: number }).version = 2;

  assert.equal(clone.name, 'terrain_copy3');
  assert.equal(source.tiles?.[0]?.description, '');
  assert.deepEqual(source.extension, { version: 1 });
});

test('fits metadata to image dimensions while retaining existing tile objects', () => {
  const source = {
    ...createDefaultTileset('sheet'),
    imageWidth: 56,
    imageHeight: 64,
    tiles: [
      {
        ...createDefaultTileset().tiles![0]!,
        id: 12,
        extension: 'kept',
      },
    ],
  };
  const expanded = reconcileTilesToDimensions(source);

  assert.equal(expanded.tiles?.length, 4);
  assert.equal(expanded.tiles?.[0]?.id, 12);
  assert.equal(expanded.tiles?.[0]?.extension, 'kept');
  assert.deepEqual(
    expanded.tiles?.slice(1).map(({ id }) => id),
    [1, 2, 3],
  );

  const shrunk = reconcileTilesToDimensions({
    ...expanded,
    imageWidth: 28,
    imageHeight: 32,
  });
  assert.equal(shrunk.tiles?.length, 1);
  assert.equal(shrunk.tiles?.[0]?.id, 12);
});

test('decodes loader-compatible step sound storage for the select control', () => {
  assert.equal(stepSoundNumber('3'), 3);
  assert.equal(stepSoundNumber(2), 2);
  assert.equal(stepSoundNumber(null), 0);
  assert.equal(stepSoundNumber('future'), 0);
  assert.equal(stepSoundNumber(99), 0);
});
