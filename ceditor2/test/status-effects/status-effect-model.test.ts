import assert from 'node:assert/strict';
import test from 'node:test';

import {
  cloneStatusEffectRecord,
  createDefaultDurationScale,
  createDefaultStatusAction,
  createStatusEffectRecord,
  createUniqueStatusEffectName,
  parseStatusEffectCollection,
  parseStatusEffectRecord,
  StatusEffectParseError,
} from '../../src/core/domain/statusEffects/index.js';

test('creates the minimal legacy-compatible status effect defaults', () => {
  assert.deepEqual(createStatusEffectRecord('NEW_STATUS'), {
    name: 'NEW_STATUS',
    description: '',
    baseDuration: 1,
    applyResistances: [],
    actions: [],
  });
  assert.deepEqual(createDefaultDurationScale(), {
    durationStat: 'STAT_MND',
    durationStatMult: 1,
  });
  assert.deepEqual(createDefaultStatusAction('ABILITY_ID'), {
    statusActionTargetType: 'STATUS_ACTION_TARGET_SELF',
    abilityName: 'ABILITY_ID',
    events: [
      {
        type: 'STATUS_EVENT_ON_APPLIED',
        condition: 'CONDITION_ALWAYS',
      },
    ],
  });
});

test('parses and detaches a complete record without dropping unknown fields', () => {
  const input = {
    name: 'BURNING',
    description: 'Burns each turn.',
    baseDuration: 3,
    futureTopLevel: { enabled: true },
    durationScale: {
      durationStat: 'STAT_MND',
      durationStatMult: 2,
      futureScaleField: 'kept',
    },
    applyBonuses: { STR: 1, futureStat: 9 },
    applyCurrentStatChange: { HP: -2 },
    applyResistances: [
      {
        attackType: 'DAMAGE_TYPE_HEAT',
        mod: 1,
        futureResistanceField: ['kept'],
      },
    ],
    actions: [
      {
        statusActionTargetType: 'STATUS_ACTION_TARGET_SELF',
        abilityName: 'SE_BURNING_1',
        futureActionField: 4,
        events: [
          {
            type: 'STATUS_EVENT_ON_TURN_START',
            condition: 'CONDITION_ALWAYS',
            futureEventField: false,
          },
        ],
      },
    ],
  };

  const parsed = parseStatusEffectRecord(input);
  input.futureTopLevel.enabled = false;
  input.actions[0]!.events[0]!.futureEventField = true;

  assert.deepEqual(parsed.futureTopLevel, { enabled: true });
  assert.equal(parsed.durationScale?.futureScaleField, 'kept');
  assert.deepEqual(parsed.applyResistances?.[0]?.futureResistanceField, [
    'kept',
  ]);
  assert.equal(parsed.actions?.[0]?.futureActionField, 4);
  assert.equal(parsed.actions?.[0]?.events[0]?.futureEventField, false);
});

test('parses a collection and reports the failing record path', () => {
  assert.throws(
    () =>
      parseStatusEffectCollection([
        createStatusEffectRecord('VALID'),
        { name: 'INVALID', description: '', baseDuration: 1.5 },
      ]),
    (error: unknown) => {
      assert.ok(error instanceof StatusEffectParseError);
      assert.equal(error.path, 'statusEffects[1].baseDuration');
      return true;
    },
  );
});

test('rejects legacy top-level fields rejected by the C++ loader', () => {
  assert.throws(
    () =>
      parseStatusEffectRecord({
        ...createStatusEffectRecord('OLD'),
        duration: 3,
      }),
    /statusEffect\.duration: legacy field is not supported/,
  );
});

test('rejects enum values that the current game loader cannot read', () => {
  assert.throws(
    () =>
      parseStatusEffectRecord({
        ...createStatusEffectRecord('BAD_CONDITION'),
        actions: [
          {
            ...createDefaultStatusAction('ABILITY'),
            events: [
              {
                type: 'STATUS_EVENT_ON_APPLIED',
                condition: 'CONDITION_IS_EVEN_ROUND',
              },
            ],
          },
        ],
      }),
    /statusEffect\.actions\[0\]\.events\[0\]\.condition: unsupported value/,
  );
});

test('generates deterministic collision-free clone names', () => {
  assert.equal(
    createUniqueStatusEffectName('BURNING', ['BURNING', 'BURNING_copy']),
    'BURNING_copy2',
  );
  assert.equal(
    createUniqueStatusEffectName('BURNING', [
      'BURNING_copy',
      'BURNING_copy2',
      'BURNING_copy3',
    ]),
    'BURNING_copy4',
  );
  assert.equal(createUniqueStatusEffectName('  ', []), 'STATUS_EFFECT_copy');
});

test('clones deeply, renames uniquely, and leaves the source untouched', () => {
  const source = parseStatusEffectRecord({
    ...createStatusEffectRecord('BURNING'),
    extension: { version: 2 },
    actions: [createDefaultStatusAction('SE_BURNING_1')],
  });

  const clone = cloneStatusEffectRecord(source, ['BURNING', 'BURNING_copy']);
  (clone.extension as { version: number }).version = 3;
  clone.actions![0]!.events[0]!.condition = 'CONDITION_ATTACK_MISSED';

  assert.equal(clone.name, 'BURNING_copy2');
  assert.deepEqual(source.extension, { version: 2 });
  assert.equal(source.actions?.[0]?.events[0]?.condition, 'CONDITION_ALWAYS');
});
