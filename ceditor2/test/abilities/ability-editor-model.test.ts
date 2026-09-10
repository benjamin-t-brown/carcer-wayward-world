import assert from 'node:assert/strict';
import test from 'node:test';

import {
  findDeepLinkedAbility,
  matchesAbilitySearch,
  abilityNameFromUrl,
  withAbilitySelection,
} from '../../src/apps/abilities/editorModel.js';
import { createDefaultAbilityRecord } from '../../src/core/domain/abilities/index.js';

const fire = {
  ...createDefaultAbilityRecord('FIRE_BOLT'),
  label: 'Fire Bolt',
  description: 'A ranged flame spell',
};
const heal = {
  ...createDefaultAbilityRecord('HEAL'),
  label: 'Mend',
  description: 'Restore health',
};

test('searches ability IDs, labels, and descriptions case-insensitively', () => {
  assert.equal(matchesAbilitySearch(fire, 'fire_b'), true);
  assert.equal(matchesAbilitySearch(fire, 'BOLT'), true);
  assert.equal(matchesAbilitySearch(fire, 'FLAME SPELL'), true);
  assert.equal(matchesAbilitySearch(fire, 'health'), false);
  assert.equal(matchesAbilitySearch(fire, '  '), true);
});

test('reads current and legacy deep-link forms', () => {
  assert.equal(
    abilityNameFromUrl(
      new URL('http://localhost/pages/abilities/?ability=HEAL'),
    ),
    'HEAL',
  );
  assert.equal(
    abilityNameFromUrl(
      new URL('http://localhost/#/abilityTemplates?selected=FIRE_BOLT'),
    ),
    'FIRE_BOLT',
  );
  assert.equal(abilityNameFromUrl(new URL('http://localhost/#HEAL')), 'HEAL');
  assert.equal(
    findDeepLinkedAbility(
      [fire, heal],
      new URL('http://localhost/?ability=HEAL'),
    ),
    1,
  );
});

test('writes canonical selection while retaining unrelated URL state', () => {
  const next = withAbilitySelection(
    new URL('http://localhost/pages/abilities/?debug=1&selected=OLD#details'),
    'FIRE BOLT',
  );
  assert.equal(next.searchParams.get('debug'), '1');
  assert.equal(next.searchParams.get('selected'), null);
  assert.equal(next.searchParams.get('ability'), 'FIRE BOLT');
  assert.equal(next.hash, '#details');
});
