import assert from 'node:assert/strict';
import test from 'node:test';

import {
  findDeepLinkedSpell,
  matchesSpellSearch,
  spellNameFromUrl,
  withSpellSelection,
} from '../../src/apps/spells/editorModel.js';
import type { SpellRecord } from '../../src/core/domain/spells/index.js';

const records: SpellRecord[] = [
  {
    name: 'FLAME',
    label: 'Flame',
    description: 'A small ball of fire.',
    icon: 'runes_0',
    abilityName: 'SPELL_FLAME',
    requiredRunes: [{ type: 'HEAT', count: 1 }],
  },
  {
    name: 'HEAL_SELF_MINOR',
    label: 'Minor Heal Self',
    description: 'Restore health.',
    icon: 'runes_2',
    abilityName: 'SPELL_HEAL_SELF_MINOR',
    requiredRunes: [{ type: 'REGROWTH', count: 1 }],
  },
];

test('spell search covers display text, IDs, and ability references', () => {
  assert.equal(matchesSpellSearch(records[0]!, 'flame'), true);
  assert.equal(matchesSpellSearch(records[0]!, 'BALL OF FIRE'), true);
  assert.equal(matchesSpellSearch(records[1]!, 'spell_heal'), true);
  assert.equal(matchesSpellSearch(records[0]!, 'regrowth'), false);
  assert.equal(matchesSpellSearch(records[0]!, '  '), true);
});

test('reads canonical and legacy spell deep links', () => {
  assert.equal(
    spellNameFromUrl(new URL('http://localhost/pages/spells/?spell=FLAME')),
    'FLAME',
  );
  assert.equal(
    spellNameFromUrl(
      new URL('http://localhost/#/editor/spellTemplates?spell=HEAL_SELF_MINOR'),
    ),
    'HEAL_SELF_MINOR',
  );
  assert.equal(
    spellNameFromUrl(new URL('http://localhost/pages/spells/#FLAME')),
    'FLAME',
  );
});

test('finds and writes a stable spell selection', () => {
  assert.equal(
    findDeepLinkedSpell(records, new URL('http://localhost/?spell=FLAME')),
    0,
  );
  assert.equal(
    findDeepLinkedSpell(records, new URL('http://localhost/?spell=MISSING')),
    -1,
  );

  const next = withSpellSelection(
    new URL('http://localhost/pages/spells/?debug=1&selected=OLD#section'),
    'HEAL SELF',
  );
  assert.equal(next.searchParams.get('debug'), '1');
  assert.equal(next.searchParams.get('selected'), null);
  assert.equal(next.searchParams.get('spell'), 'HEAL SELF');
  assert.equal(next.hash, '#section');
  assert.equal(withSpellSelection(next).searchParams.get('spell'), null);
});
