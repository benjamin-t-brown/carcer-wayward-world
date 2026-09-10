import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import {
  ItemParseError,
  changeItemType,
  changeItemUsability,
  cloneItemRecord,
  createDefaultItemRecord,
  createUniqueItemName,
  isItemUsable,
  itemStatusEffectName,
  parseItemCollection,
  parseItemRecord,
  runeIcon,
} from '../../src/core/domain/items/index.js';

test('creates small loader-compatible defaults', () => {
  assert.deepEqual(createDefaultItemRecord('NEW_ITEM'), {
    itemType: 'UTILITY',
    name: 'NEW_ITEM',
    label: '',
    icon: 'ui_item_icons_0',
    description: '',
    weight: 1,
    value: 1,
    stackable: false,
    indestructable: false,
    itemUsability: 'NOT_USABLE',
    statusEffects: [],
  });
});

test('parses the live item database', async () => {
  const source = JSON.parse(
    await readFile(
      new URL('../../../src/assets/db/items.json', import.meta.url),
      'utf8',
    ),
  ) as unknown;
  const items = parseItemCollection(source);
  assert.ok(items.length > 0);
  assert.ok(items.some((item) => item.itemType === 'WEAPON_MELEE'));
});

test('detaches records while retaining unknown fields and reference shapes', () => {
  const input = {
    ...createDefaultItemRecord('FUTURE_ITEM'),
    future: { enabled: true },
    itemUsability: 'USABLE_EVERYWHERE',
    statusEffects: ['BURNING', { name: 'FROZEN', futureReference: ['kept'] }],
    useAbility: {
      abilityName: 'SPELL_HEAL',
      futureConfig: 4,
      restoreOverrides: [
        {
          restoreWhich: 'CURRENT_STAT_HP',
          restoreDice: ['D6'],
          restoreBonus: 2,
          restoreStat: 'STAT_CON',
          restoreStatMult: 1,
          futureRestore: false,
        },
      ],
    },
  };
  const parsed = parseItemRecord(input);
  input.future.enabled = false;
  const objectReference = input.statusEffects[1];
  if (typeof objectReference !== 'string') {
    objectReference!.futureReference.push('changed');
  }

  assert.deepEqual(parsed.future, { enabled: true });
  assert.equal(parsed.useAbility?.futureConfig, 4);
  assert.equal(parsed.useAbility?.restoreOverrides?.[0]?.futureRestore, false);
  assert.deepEqual(parsed.statusEffects?.[1], {
    name: 'FROZEN',
    futureReference: ['kept'],
  });
  assert.equal(itemStatusEffectName(parsed.statusEffects![0]!), 'BURNING');
  assert.equal(itemStatusEffectName(parsed.statusEffects![1]!), 'FROZEN');
});

test('matches the game loader rune constraint and reports precise paths', () => {
  assert.throws(
    () =>
      parseItemRecord({ ...createDefaultItemRecord('BAD'), itemType: 'RUNE' }),
    (error: unknown) => {
      assert.ok(error instanceof ItemParseError);
      assert.equal(error.path, 'item.runeType');
      return true;
    },
  );
  assert.throws(
    () =>
      parseItemRecord({
        ...createDefaultItemRecord('BAD'),
        runeType: 'HEAT',
      }),
    /item\.runeType: non-RUNE items must not set runeType/,
  );
  assert.throws(
    () =>
      parseItemCollection([
        createDefaultItemRecord('SAME'),
        createDefaultItemRecord('SAME'),
      ]),
    /items\[1\]\.name: duplicate item "SAME"/,
  );
});

test('retains unknown usability strings because the game loader accepts them', () => {
  const parsed = parseItemRecord({
    ...createDefaultItemRecord('FUTURE_USABILITY'),
    itemUsability: 'USABLE_IN_DREAMS',
  });
  assert.equal(parsed.itemUsability, 'USABLE_IN_DREAMS');
  assert.equal(isItemUsable(parsed.itemUsability), false);
});

test('explicit type and usability changes update only their dependent sections', () => {
  const source = parseItemRecord({
    ...createDefaultItemRecord('ITEM'),
    extension: { retained: true },
  });
  const rune = changeItemType(source, 'RUNE');
  assert.equal(rune.runeType, 'HEAT');
  assert.equal(rune.icon, 'ui_item_icons_0');
  assert.deepEqual(rune.extension, { retained: true });
  assert.equal(runeIcon('COMPACT'), 'runes_7');

  const weapon = changeItemType(rune, 'WEAPON_RANGED');
  assert.deepEqual(weapon.weapon, { abilityName: '' });
  assert.equal(weapon.runeType, undefined);

  const usable = changeItemUsability(weapon, 'USABLE_COMBAT_ONLY');
  assert.deepEqual(usable.useAbility, { abilityName: '' });
  const disabled = changeItemUsability(usable, 'NOT_USABLE');
  assert.equal(disabled.useAbility, undefined);
  assert.equal(disabled.useSpecialEvent, undefined);
});

test('clones deeply with deterministic collision-free names', () => {
  assert.equal(
    createUniqueItemName('Potion', ['Potion', 'Potion_copy', 'Potion_copy2']),
    'Potion_copy3',
  );
  const source = parseItemRecord({
    ...createDefaultItemRecord('Potion'),
    extension: { count: 1 },
  });
  const clone = cloneItemRecord(source, ['Potion', 'Potion_copy']);
  (clone.extension as { count: number }).count = 2;
  assert.equal(clone.name, 'Potion_copy2');
  assert.deepEqual(source.extension, { count: 1 });
});
