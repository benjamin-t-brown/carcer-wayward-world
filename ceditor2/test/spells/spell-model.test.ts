import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import {
  cloneSpellRecord,
  createSpellRecord,
  createUniqueSpellName,
  parseSpellCollection,
  parseSpellRecord,
  SpellParseError,
} from '../../src/core/domain/spells/index.js';

test('creates loader-compatible spell defaults', () => {
  assert.deepEqual(createSpellRecord('NEW_SPELL'), {
    name: 'NEW_SPELL',
    label: '',
    description: '',
    icon: 'runes_0',
    abilityName: '',
    requiredRunes: [],
  });
});

test('parses and detaches records without changing order or unknown fields', () => {
  const input = {
    name: 'FIRE_WALL',
    label: 'Fire Wall',
    description: 'A wall of flame.',
    icon: 'runes_0',
    abilityName: 'SPELL_FIRE_WALL',
    futureField: { version: 2 },
    requiredRunes: [
      { type: 'HEAT', count: 2, futureRuneField: 'kept' },
      { type: 'EXPAND', count: 1 },
    ],
  };

  const parsed = parseSpellRecord(input);
  input.futureField.version = 3;
  input.requiredRunes[0]!.count = 8;

  assert.deepEqual(Object.keys(parsed), Object.keys(input));
  assert.equal((parsed.futureField as { version: number }).version, 2);
  assert.equal(parsed.requiredRunes[0]?.count, 2);
  assert.equal(parsed.requiredRunes[0]?.futureRuneField, 'kept');
  assert.deepEqual(
    parsed.requiredRunes.map((requirement) => requirement.type),
    ['HEAT', 'EXPAND'],
  );
});

test('parses the live spells database against the current loader contract', async () => {
  const source = await readFile('../src/assets/db/spells.json', 'utf8');
  const records = parseSpellCollection(JSON.parse(source));
  assert.ok(records.length > 0);
  assert.ok(records.every((record) => record.name && record.abilityName));
});

test('reports precise required rune validation paths', () => {
  const base = createSpellRecord('BAD');
  assert.throws(
    () =>
      parseSpellRecord({
        ...base,
        requiredRunes: [{ type: 'HEAT', count: 0 }],
      }),
    (error: unknown) => {
      assert.ok(error instanceof SpellParseError);
      assert.equal(error.path, 'spell.requiredRunes[0].count');
      return true;
    },
  );
  assert.throws(
    () =>
      parseSpellRecord({
        ...base,
        requiredRunes: [
          { type: 'HEAT', count: 1 },
          { type: 'HEAT', count: 2 },
        ],
      }),
    /spell\.requiredRunes\[1\]\.type: duplicate rune type/,
  );
  assert.throws(
    () =>
      parseSpellRecord({
        ...base,
        requiredRunes: [{ type: 'LIGHT', count: 1 }],
      }),
    /spell\.requiredRunes\[0\]\.type: unsupported value/,
  );
});

test('rejects duplicate spell names in a collection', () => {
  assert.throws(
    () =>
      parseSpellCollection([
        createSpellRecord('SAME'),
        createSpellRecord('SAME'),
      ]),
    /spells\[1\]\.name: duplicate spell name/,
  );
});

test('clones deeply with a deterministic collision-free name', () => {
  assert.equal(
    createUniqueSpellName('FLAME', ['FLAME_copy', 'FLAME_copy2']),
    'FLAME_copy3',
  );
  const source = parseSpellRecord({
    ...createSpellRecord('FLAME'),
    extension: { enabled: true },
    requiredRunes: [{ type: 'HEAT', count: 1 }],
  });
  const clone = cloneSpellRecord(source, ['FLAME', 'FLAME_copy']);
  clone.requiredRunes[0]!.count = 3;
  (clone.extension as { enabled: boolean }).enabled = false;

  assert.equal(clone.name, 'FLAME_copy2');
  assert.equal(source.requiredRunes[0]?.count, 1);
  assert.deepEqual(source.extension, { enabled: true });
});
