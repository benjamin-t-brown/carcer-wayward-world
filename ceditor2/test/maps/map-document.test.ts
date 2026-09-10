import assert from 'node:assert/strict';
import test from 'node:test';

import {
  MapDocument,
  MapParseError,
  parseMapCollection,
  parseMapRecord,
  type MapRecord,
} from '../../src/core/domain/maps/index.js';

function mapFixture(): MapRecord {
  return {
    name: 'UNDERGROUND_TEST',
    label: 'Underground Test',
    type: 'TOWN',
    width: 2,
    height: 2,
    spriteWidth: 28,
    spriteHeight: 32,
    tilesets: ['', 'terrain0'],
    layers: [1, 0, -1],
    tiles: {
      '1': [0, 0, 0, 0, 0, 0, 0, 0],
      '0': [1, 10, 1, 11, 1, 12, 1, 13],
      '-1': [1, 20, 1, 21, 1, 22, 1, 23],
    },
    characters: [{ l: 0, i: 1, name: 'GUARD', futurePlacement: 'kept' }],
    futureMapField: { version: 2 },
  };
}

test('parser preserves negative layers, unknown fields, order, and omissions', () => {
  const input = mapFixture();
  const parsed = parseMapRecord(input);
  input.layers.reverse();
  input.characters![0]!.futurePlacement = 'changed';

  assert.deepEqual(parsed.layers, [1, 0, -1]);
  assert.deepEqual(parsed.tiles['-1'], [1, 20, 1, 21, 1, 22, 1, 23]);
  assert.equal(parsed.characters?.[0]?.futurePlacement, 'kept');
  assert.deepEqual(parsed.futureMapField, { version: 2 });
  assert.equal('items' in parsed, false);
});

test('collection parser reports the exact malformed dense layer path', () => {
  const invalid = mapFixture();
  invalid.tiles['-1'] = [1, 20];

  assert.throws(
    () => parseMapCollection([mapFixture(), invalid]),
    (error: unknown) => {
      assert.ok(error instanceof MapParseError);
      assert.equal(error.path, 'maps[1].tiles["-1"]');
      assert.match(error.message, /expected 8 entries, received 2/);
      return true;
    },
  );
});

test('document exposes stable dimensions, layer data, and tileset names', () => {
  const document = MapDocument.from(mapFixture());

  assert.equal(document.name, 'UNDERGROUND_TEST');
  assert.equal(document.width, 2);
  assert.equal(document.height, 2);
  assert.equal(document.cellCount, 4);
  assert.equal(document.spriteWidth, 28);
  assert.equal(document.spriteHeight, 32);
  assert.deepEqual(document.layers, [1, 0, -1]);
  assert.deepEqual(document.tilesetNames, ['', 'terrain0']);
  assert.deepEqual(document.layerData(-1), [1, 20, 1, 21, 1, 22, 1, 23]);
  assert.equal(document.hasLayer(-1), true);
});

test('sprite getters apply runtime defaults without writing them to snapshots', () => {
  const fixture = mapFixture();
  delete fixture.spriteWidth;
  delete fixture.spriteHeight;
  const document = MapDocument.from(fixture);

  assert.equal(document.spriteWidth, 28);
  assert.equal(document.spriteHeight, 32);
  assert.equal('spriteWidth' in document.snapshot(), false);
  assert.equal('spriteHeight' in document.snapshot(), false);
});

test('parser rejects non-positive explicit sprite dimensions', () => {
  const zeroWidth = mapFixture();
  zeroWidth.spriteWidth = 0;
  assert.throws(
    () => parseMapRecord(zeroWidth, 'maps[3]'),
    (error: unknown) => {
      assert.ok(error instanceof MapParseError);
      assert.equal(error.path, 'maps[3].spriteWidth');
      return true;
    },
  );

  const negativeHeight = mapFixture();
  negativeHeight.spriteHeight = -1;
  assert.throws(() => parseMapRecord(negativeHeight), /positive integer/);
});

test('coordinate and cell reads are bounded and support negative layers', () => {
  const document = MapDocument.from(mapFixture());

  assert.equal(document.indexAt(1, 1), 3);
  assert.deepEqual(document.coordinatesOf(2), { x: 0, y: 1 });
  assert.deepEqual(document.readCell(0, 2), {
    tilesetIndex: 1,
    tileIndex: 12,
  });
  assert.deepEqual(document.readCellAt(-1, 1, 0), {
    tilesetIndex: 1,
    tileIndex: 21,
  });
  assert.equal(document.indexAt(2, 0), undefined);
  assert.equal(document.indexAt(0.5, 0), undefined);
  assert.equal(document.coordinatesOf(-1), undefined);
  assert.equal(document.readCell(9, 0), undefined);
  assert.equal(document.readCell(0, 4), undefined);
});

test('bounded writes mutate only the document-owned dense pair', () => {
  const fixture = mapFixture();
  const document = MapDocument.from(fixture);

  assert.equal(
    document.writeCellAt(-1, 1, 1, { tilesetIndex: 3, tileIndex: 99 }),
    true,
  );
  assert.equal(
    document.writeCell(0, 4, { tilesetIndex: 8, tileIndex: 8 }),
    false,
  );
  assert.equal(
    document.writeCell(7, 0, { tilesetIndex: 8, tileIndex: 8 }),
    false,
  );

  assert.deepEqual(document.readCell(-1, 3), {
    tilesetIndex: 3,
    tileIndex: 99,
  });
  assert.deepEqual(fixture.tiles['-1'], [1, 20, 1, 21, 1, 22, 1, 23]);
  assert.equal(document.snapshot().characters?.[0]?.futurePlacement, 'kept');
});

test('compact patches apply in order while safely rejecting invalid targets', () => {
  const document = MapDocument.from(mapFixture());
  const applied = document.applyCellPatches([
    { layer: 0, index: 0, tilesetIndex: 2, tileIndex: 40 },
    { layer: -1, index: 2, tilesetIndex: 2, tileIndex: 41 },
    { layer: -2, index: 0, tilesetIndex: 2, tileIndex: 42 },
    { layer: 0, index: 8, tilesetIndex: 2, tileIndex: 43 },
  ]);

  assert.equal(applied, 2);
  assert.deepEqual(document.readCell(0, 0), {
    tilesetIndex: 2,
    tileIndex: 40,
  });
  assert.deepEqual(document.readCell(-1, 2), {
    tilesetIndex: 2,
    tileIndex: 41,
  });
  assert.deepEqual(document.placements.characters, [
    { l: 0, i: 1, name: 'GUARD', futurePlacement: 'kept' },
  ]);
});
