import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';

import {
  CharacterParseError,
  cloneCharacterRecord,
  createDefaultCharacterRecord,
  createDefaultCharacterStats,
  createUniqueCharacterName,
  parseCharacterCollection,
  parseCharacterRecord,
} from '../../src/core/domain/characters/index.js';

test('creates a complete loader-compatible character default', () => {
  const record = createDefaultCharacterRecord('NEW_CHARACTER');
  assert.equal(record.name, 'NEW_CHARACTER');
  assert.equal(record.type, 'TOWNSPERSON');
  assert.equal(record.spritesheet, 'actors0');
  assert.equal(record.spriteOffset, 0);
  assert.deepEqual(record.stats, createDefaultCharacterStats());
});

test('parses, detaches, and preserves unknown data at every known level', () => {
  const input = {
    ...createDefaultCharacterRecord('keeper'),
    future: { flag: true },
    stats: {
      futureStats: 1,
      generic: { str: 2, futureGeneric: [1] },
      trainable: {
        futureTrainable: false,
        weapon: { edged: 3, futureWeapon: 'kept' },
      },
      skills: { social: 4, futureSkill: null },
    },
    talk: { talkName: 'hello', futureTalk: 5 },
    behavior: { behaviorName: 'IMMOBILE', futureBehavior: true },
    combat: { hp: 8, stats: { con: 7, legacyFuture: 'kept' }, futureCombat: 2 },
    combatBehavior: { town: 'SEEK_AND_MELEE', futureAi: [2] },
    sound: { deathSound: 'legacy', futureSound: false },
    statuses: [{ status: 'BURNING', futureStatus: { amount: 1 } }],
    vision: { radius: 6, futureVision: 'night' },
  };

  const parsed = parseCharacterRecord(input);
  input.future.flag = false;
  input.statuses[0]!.futureStatus.amount = 9;

  assert.deepEqual(parsed.future, { flag: true });
  assert.deepEqual(parsed.stats?.generic?.futureGeneric, [1]);
  assert.equal(parsed.stats?.trainable?.weapon?.futureWeapon, 'kept');
  assert.equal(parsed.talk?.futureTalk, 5);
  assert.equal(parsed.behavior?.futureBehavior, true);
  assert.equal(parsed.combat?.stats?.legacyFuture, 'kept');
  assert.deepEqual(parsed.combatBehavior?.futureAi, [2]);
  assert.equal(parsed.sound?.futureSound, false);
  assert.deepEqual(parsed.statuses?.[0]?.futureStatus, { amount: 1 });
  assert.equal(parsed.vision?.futureVision, 'night');
});

test('accepts both sprite offset representations and legacy loader fields', () => {
  const numeric = parseCharacterRecord(createDefaultCharacterRecord('numeric'));
  const named = parseCharacterRecord({
    ...createDefaultCharacterRecord('named'),
    spriteOffset: 'idle_south',
    combat: { stats: { str: 9 } },
    sound: { deathSound: 'old_death', weaponSound: 'old_weapon' },
  });
  assert.equal(numeric.spriteOffset, 0);
  assert.equal(named.spriteOffset, 'idle_south');
  assert.equal(named.combat?.stats?.str, 9);
  assert.equal(named.sound?.deathSound, 'old_death');
});

test('reports precise invalid nested paths', () => {
  const invalid = {
    ...createDefaultCharacterRecord('bad'),
    stats: { trainable: { body: { dr: 1.5 } } },
  };
  assert.throws(
    () => parseCharacterRecord(invalid),
    (error: unknown) => {
      assert.ok(error instanceof CharacterParseError);
      assert.equal(error.path, 'character.stats.trainable.body.dr');
      return true;
    },
  );
});

test('rejects duplicate character names', () => {
  assert.throws(
    () =>
      parseCharacterCollection([
        createDefaultCharacterRecord('same'),
        createDefaultCharacterRecord('same'),
      ]),
    /characters\[1\]\.name: duplicate character "same"/,
  );
});

test('clones deeply with deterministic collision-free IDs', () => {
  assert.equal(
    createUniqueCharacterName('npc', ['npc_copy', 'npc_copy2']),
    'npc_copy3',
  );
  const source = parseCharacterRecord({
    ...createDefaultCharacterRecord('npc'),
    future: { version: 1 },
    statuses: [{ status: 'BURNING', future: true }],
  });
  const clone = cloneCharacterRecord(source, ['npc', 'npc_copy']);
  (clone.future as { version: number }).version = 2;
  clone.statuses![0]!.status = 'FROZEN';
  assert.equal(clone.name, 'npc_copy2');
  assert.deepEqual(source.future, { version: 1 });
  assert.equal(source.statuses?.[0]?.status, 'BURNING');
});

test('loads the real game character database without normalization', () => {
  const source = JSON.parse(
    readFileSync(
      new URL('../../../src/assets/db/characters.json', import.meta.url),
      'utf8',
    ),
  ) as unknown;
  assert.deepEqual(parseCharacterCollection(source), source);
});
