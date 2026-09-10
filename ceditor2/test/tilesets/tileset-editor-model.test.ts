import assert from 'node:assert/strict';
import test from 'node:test';

import {
  findDeepLinkedTileset,
  matchesTilesetSearch,
  tilesetNameFromUrl,
  withTilesetSelection,
} from '../../src/apps/tilesets/editorModel.js';
import { createDefaultTileset } from '../../src/core/domain/tilesets/index.js';

const terrain = {
  ...createDefaultTileset('terrain0'),
  spriteBase: 'world_terrain',
};
const doors = {
  ...createDefaultTileset('interior_doors'),
  spriteBase: 'props',
};

test('searches names and sprite bases case-insensitively', () => {
  assert.equal(matchesTilesetSearch(terrain, 'TERRAIN0'), true);
  assert.equal(matchesTilesetSearch(terrain, 'world_'), true);
  assert.equal(matchesTilesetSearch(terrain, 'doors'), false);
  assert.equal(matchesTilesetSearch(terrain, '  '), true);
});

test('reads canonical and legacy deep links', () => {
  assert.equal(
    tilesetNameFromUrl(new URL('http://localhost/?tileset=terrain0')),
    'terrain0',
  );
  assert.equal(
    tilesetNameFromUrl(
      new URL('http://localhost/#/tilesets?selected=interior_doors'),
    ),
    'interior_doors',
  );
  assert.equal(
    findDeepLinkedTileset(
      [terrain, doors],
      new URL('http://localhost/?selected=interior_doors'),
    ),
    1,
  );
});

test('writes canonical selection and keeps unrelated URL state', () => {
  const next = withTilesetSelection(
    new URL('http://localhost/?debug=1&selected=old#details'),
    'world tiles',
  );
  assert.equal(next.searchParams.get('debug'), '1');
  assert.equal(next.searchParams.get('selected'), null);
  assert.equal(next.searchParams.get('tileset'), 'world tiles');
  assert.equal(next.hash, '#details');
});
