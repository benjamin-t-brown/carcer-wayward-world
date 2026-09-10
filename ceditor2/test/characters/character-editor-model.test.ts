import assert from 'node:assert/strict';
import test from 'node:test';

import {
  characterNameFromUrl,
  findDeepLinkedCharacter,
  matchesCharacterSearch,
  withCharacterSelection,
} from '../../src/apps/characters/editorModel.js';
import { createDefaultCharacterRecord } from '../../src/core/domain/characters/index.js';

const claire = {
  ...createDefaultCharacterRecord('alinea_Claire'),
  label: 'Dockmaster Claire',
};
const goblin = {
  ...createDefaultCharacterRecord('goblinTest'),
  label: 'Goblin Test',
  type: 'ENEMY' as const,
};

test('searches character IDs, labels, and types case-insensitively', () => {
  assert.equal(matchesCharacterSearch(claire, 'alinea_cl'), true);
  assert.equal(matchesCharacterSearch(claire, 'DOCKMASTER'), true);
  assert.equal(matchesCharacterSearch(goblin, 'enemy'), true);
  assert.equal(matchesCharacterSearch(goblin, 'townsperson_static'), false);
  assert.equal(matchesCharacterSearch(goblin, '  '), true);
});

test('reads current and legacy character deep links', () => {
  assert.equal(
    characterNameFromUrl(
      new URL('http://localhost/pages/characters/?character=goblinTest'),
    ),
    'goblinTest',
  );
  assert.equal(
    characterNameFromUrl(
      new URL(
        'http://localhost/#/editor/characterTemplates?selected=alinea_Claire',
      ),
    ),
    'alinea_Claire',
  );
  assert.equal(
    characterNameFromUrl(
      new URL('http://localhost/pages/characters/#goblinTest'),
    ),
    'goblinTest',
  );
  assert.equal(
    findDeepLinkedCharacter(
      [claire, goblin],
      new URL('http://localhost/?character=goblinTest'),
    ),
    1,
  );
});

test('writes canonical selection while retaining unrelated URL state', () => {
  const next = withCharacterSelection(
    new URL('http://localhost/pages/characters/?debug=1&selected=OLD#details'),
    'npc one',
  );
  assert.equal(next.searchParams.get('debug'), '1');
  assert.equal(next.searchParams.get('selected'), null);
  assert.equal(next.searchParams.get('character'), 'npc one');
  assert.equal(next.hash, '#details');
});
