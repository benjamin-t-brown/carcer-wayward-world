import assert from 'node:assert/strict';
import test from 'node:test';

import type { AbilityTemplate } from '../client/types/ability';
import { sanitizeAbilityTemplates } from '../client/types/ability';
import type { ItemTemplate, MapGridTemplate } from '../client/types/assets';
import {
  normalizeItemUseAbilityConfig,
  sanitizeItemTemplates,
  sanitizeMapGridTemplates,
} from '../client/types/assets';
import type { SpellTemplate } from '../client/types/spell';
import { sanitizeSpellTemplates } from '../client/types/spell';
import { trimStrings } from '../client/utils/jsonUtils';

test('trimStrings recursively trims a clone without losing unknown fields', () => {
  const input = {
    name: '  known  ',
    futureMetadata: {
      note: '  retained  ',
      values: [' first ', 2, true, null],
    },
  };
  const before = structuredClone(input);

  const prepared = trimStrings(input);

  assert.deepEqual(prepared, {
    name: 'known',
    futureMetadata: {
      note: 'retained',
      values: ['first', 2, true, null],
    },
  });
  assert.deepEqual(input, before);
  assert.notEqual(prepared, input);
  assert.notEqual(prepared.futureMetadata, input.futureMetadata);
  assert.notEqual(prepared.futureMetadata.values, input.futureMetadata.values);
});

test('ability normalization is deterministic, immutable, and forward-compatible', () => {
  const input = [
    {
      name: 'strike',
      label: 'Strike',
      description: '',
      icon: 'icon',
      type: 'ABILITY_ATTACK',
      targetSelect: {
        targetType: 'TARGET_MOVE',
        allegianceSelectType: 'TARGET_ALLEGIANCE_OTHER',
        numTargetableUnits: 1,
        zoneSize: { x: 1, y: 1 },
        range: 2,
        futureTargetRule: 'keep-me',
      },
      apCost: 1,
      costType: 'ABILITY_COST_NONE',
      costValue: 0,
      depiction: {
        dmgAnim: 'known-animation',
        projectileType: 'PROJECTILE_NONE',
        projectilePath: 'PROJECTILE_PATH_NONE',
        startSound: 'missing-sound',
        dmgSound: 'known-sound',
        futureDepictionRule: 'keep-me-too',
      },
      futureAbilityRule: { enabled: true },
    } as unknown as AbilityTemplate,
  ];
  const before = structuredClone(input);
  const animationMap = { 'known-animation': {} };
  const soundMap = { 'known-sound': {} };

  const first = sanitizeAbilityTemplates(input, animationMap, soundMap);
  const second = sanitizeAbilityTemplates(input, animationMap, soundMap);

  assert.deepEqual(first, second);
  assert.deepEqual(input, before);
  assert.notEqual(first, input);
  assert.notEqual(first[0], input[0]);
  assert.equal(first[0].targetSelect.targetType, 'TARGET_UNIT');
  assert.equal(first[0].depiction.startSound, '');
  assert.equal(first[0].depiction.dmgSound, 'known-sound');
  assert.deepEqual(
    (first[0] as AbilityTemplate & { futureAbilityRule: unknown })
      .futureAbilityRule,
    { enabled: true },
  );
  assert.equal(
    (
      first[0].targetSelect as AbilityTemplate['targetSelect'] & {
        futureTargetRule: string;
      }
    ).futureTargetRule,
    'keep-me',
  );
  assert.equal(
    (
      first[0].depiction as AbilityTemplate['depiction'] & {
        futureDepictionRule: string;
      }
    ).futureDepictionRule,
    'keep-me-too',
  );
});

test('item normalization removes known legacy data without mutating or dropping unknown fields', () => {
  const input = [
    {
      itemType: 'UTILITY',
      name: 'tool',
      label: 'Tool',
      icon: 'tool-icon',
      description: '',
      weight: 1,
      value: 2,
      itemUsability: 'NOT_USABLE',
      itemUsabilityArgs: { itemUsabilityType: 'legacy' },
      futureItemRule: { version: 2 },
    } as ItemTemplate,
  ];
  const before = structuredClone(input);

  const prepared = sanitizeItemTemplates(input);

  assert.deepEqual(input, before);
  assert.notEqual(prepared, input);
  assert.notEqual(prepared[0], input[0]);
  assert.equal(prepared[0].itemUsabilityArgs, undefined);
  assert.deepEqual(
    (prepared[0] as ItemTemplate & { futureItemRule: unknown }).futureItemRule,
    { version: 2 },
  );
});

test('item ability normalization omits absent overrides', () => {
  assert.deepEqual(normalizeItemUseAbilityConfig({ abilityName: 'heal' }), {
    abilityName: 'heal',
  });
});

test('map-grid normalization preserves unknown fields', () => {
  const grid = {
    name: 'overworld',
    label: 'Overworld',
    gridWidth: 1,
    gridHeight: 1,
    mapWidth: 25,
    mapHeight: 20,
    cells: [['partition-a']],
    futureGridRule: { streamingPriority: 2 },
  } as unknown as MapGridTemplate;

  const [normalized] = sanitizeMapGridTemplates([grid]);

  assert.deepEqual(
    (
      normalized as MapGridTemplate & {
        futureGridRule: { streamingPriority: number };
      }
    ).futureGridRule,
    { streamingPriority: 2 },
  );
});

test('spell normalization is deterministic, immutable, and preserves unknown fields', () => {
  const input = [
    {
      name: 'spark',
      label: 'Spark',
      description: '',
      icon: 'runes_0',
      abilityName: 'spark-ability',
      requiredRunes: [
        { type: 'HEAT', count: 2.8 },
        { type: 'HEAT', count: 9 },
      ],
      futureSpellRule: { source: 'game' },
    } as SpellTemplate,
  ];
  const before = structuredClone(input);

  const first = sanitizeSpellTemplates(input);
  const second = sanitizeSpellTemplates(input);

  assert.deepEqual(first, second);
  assert.deepEqual(input, before);
  assert.notEqual(first, input);
  assert.notEqual(first[0], input[0]);
  assert.deepEqual(first[0].requiredRunes, [{ type: 'HEAT', count: 2 }]);
  assert.deepEqual(
    (first[0] as SpellTemplate & { futureSpellRule: unknown }).futureSpellRule,
    { source: 'game' },
  );
});
