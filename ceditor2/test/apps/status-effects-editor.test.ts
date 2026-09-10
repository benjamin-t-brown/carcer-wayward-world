import assert from 'node:assert/strict';
import test from 'node:test';
import type { StatusEffectRecord } from '../../src/core/domain/statusEffects/index.js';
import {
  findDeepLinkedStatusEffect,
  matchesStatusEffectSearch,
  statusEffectNameFromUrl,
  withStatusEffectSelection,
} from '../../src/apps/status-effects/editorModel.js';

const records: StatusEffectRecord[] = [
  {
    name: 'BURNING',
    description: 'Fire damage each turn',
    baseDuration: 3,
    applyResistances: [],
    actions: [],
  },
  {
    name: 'FROZEN',
    description: 'Stops movement',
    baseDuration: 1,
    applyResistances: [],
    actions: [],
  },
];

test('status effect search covers names and descriptions case-insensitively', () => {
  assert.equal(matchesStatusEffectSearch(records[0]!, 'burn'), true);
  assert.equal(matchesStatusEffectSearch(records[0]!, 'FIRE DAMAGE'), true);
  assert.equal(matchesStatusEffectSearch(records[0]!, 'movement'), false);
  assert.equal(matchesStatusEffectSearch(records[0]!, '  '), true);
});

test('reads canonical and legacy status effect deep links', () => {
  assert.equal(
    statusEffectNameFromUrl(
      new URL('http://localhost/pages/status-effects/?statusEffect=BURNING'),
    ),
    'BURNING',
  );
  assert.equal(
    statusEffectNameFromUrl(
      new URL(
        'http://localhost/#/editor/statusEffectTemplates?statusEffect=FROZEN',
      ),
    ),
    'FROZEN',
  );
  assert.equal(
    statusEffectNameFromUrl(
      new URL('http://localhost/pages/status-effects/#BURNING'),
    ),
    'BURNING',
  );
});

test('selects a deep-linked record by stable name', () => {
  assert.equal(
    findDeepLinkedStatusEffect(
      records,
      new URL('http://localhost/?statusEffect=FROZEN'),
    ),
    1,
  );
  assert.equal(
    findDeepLinkedStatusEffect(
      records,
      new URL('http://localhost/?statusEffect=MISSING'),
    ),
    -1,
  );
});

test('writes one canonical selection without dropping unrelated query state', () => {
  const next = withStatusEffectSelection(
    new URL(
      'http://localhost/pages/status-effects/?debug=1&selected=OLD#section',
    ),
    'BURNING FIRE',
  );
  assert.equal(next.searchParams.get('debug'), '1');
  assert.equal(next.searchParams.get('selected'), null);
  assert.equal(next.searchParams.get('statusEffect'), 'BURNING FIRE');
  assert.equal(next.hash, '#section');

  const cleared = withStatusEffectSelection(next);
  assert.equal(cleared.searchParams.get('statusEffect'), null);
});
