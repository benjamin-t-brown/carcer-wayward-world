import assert from 'node:assert/strict';
import test from 'node:test';

import {
  replaceTilePlacements,
  tilePlacementsAt,
} from '../../src/core/domain/maps/index.js';

const map = {
  name: 'map',
  width: 2,
  height: 1,
  tilesets: [''],
  layers: [0],
  tiles: { '0': [0, 0, 0, 0] },
  characters: [
    { l: 0, i: 0, name: 'old', future: true },
    { l: 0, i: 1, name: 'untouched' },
  ],
  items: [],
  futureMap: { keep: true },
};

test('tile metadata bundles include only records at the selected address', () => {
  assert.deepEqual(tilePlacementsAt(map, 0, 0), {
    characters: [{ l: 0, i: 0, name: 'old', future: true }],
  });
});

test('tile metadata replacement preserves other cells and unknown map data', () => {
  const updated = replaceTilePlacements(map, 0, 0, {
    characters: [{ name: 'new', custom: 3 }],
    items: [{ name: 'potion', quantity: 2 }],
    eventTriggers: [{ eventId: 'intro', requiresLook: true }],
  });

  assert.deepEqual(updated.characters, [
    { l: 0, i: 1, name: 'untouched' },
    { name: 'new', custom: 3, l: 0, i: 0 },
  ]);
  assert.deepEqual(updated.items, [
    { name: 'potion', quantity: 2, l: 0, i: 0 },
  ]);
  assert.deepEqual(updated.eventTriggers, [
    { eventId: 'intro', requiresLook: true, l: 0, i: 0 },
  ]);
  assert.deepEqual(updated.futureMap, { keep: true });
  assert.equal(map.characters[0]?.name, 'old');
});

test('tile metadata rejects missing layers and invalid bundles', () => {
  assert.throws(() => tilePlacementsAt(map, 2, 0), /layer 2 is missing/);
  assert.throws(
    () => replaceTilePlacements(map, 0, 0, { items: 3 } as never),
    /items must be an array/,
  );
});
