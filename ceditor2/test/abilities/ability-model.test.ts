import assert from 'node:assert/strict';
import test from 'node:test';

import {
  AbilityParseError,
  cloneAbilityRecord,
  createDefaultAbilityRecord,
  createDefaultAbilitySave,
  createDefaultAttackDamage,
  createUniqueAbilityName,
  parseAbilityCollection,
  parseAbilityRecord,
} from '../../src/core/domain/abilities/index.js';

test('creates complete loader-compatible ability defaults', () => {
  const ability = createDefaultAbilityRecord('NEW_ABILITY');
  assert.equal(ability.name, 'NEW_ABILITY');
  assert.equal(ability.type, 'ABILITY_ATTACK');
  assert.deepEqual(ability.targetSelect.zoneSize, { x: 1, y: 1 });
  assert.equal(ability.depiction.projectileType, 'PROJECTILE_NONE');
  assert.deepEqual(ability.attacks, []);
  assert.deepEqual(createDefaultAttackDamage().dmgDice, ['D6']);
  assert.deepEqual(createDefaultAbilitySave(), {
    saveStat: 'STAT_MND',
    saveBase: 0,
    saveAgainst: 'STAT_MND',
    saveAgainstBase: 0,
  });
});

test('parses, detaches, and retains unknown fields at every level', () => {
  const input = {
    ...createDefaultAbilityRecord('FIRE'),
    future: { enabled: true },
    targetSelect: {
      ...createDefaultAbilityRecord().targetSelect,
      futureTarget: 'kept',
    },
    depiction: {
      ...createDefaultAbilityRecord().depiction,
      futureDepiction: [1, 2],
    },
    attacks: [
      {
        attackClass: 'ATTACK_CLASS_MAGIC',
        futureAttack: 4,
        dmg: { ...createDefaultAttackDamage(), futureDamage: false },
        save: { ...createDefaultAbilitySave(), futureSave: 'kept' },
      },
    ],
    statuses: [{ statusEffect: 'BURNING', futureStatus: { value: 3 } }],
    restores: [
      {
        restoreWhich: 'CURRENT_STAT_HP',
        restoreDice: ['D8'],
        restoreBonus: 2,
        restoreStat: 'STAT_MND',
        restoreStatMult: 1,
        futureRestore: true,
      },
    ],
    damages: [
      {
        damageType: 'DAMAGE_TYPE_HEAT',
        dmgDice: ['D6'],
        dmgBonus: 1,
        dmgStat: 'STAT_MND',
        dmgStatMult: 0.5,
        futureDirectDamage: null,
      },
    ],
  };

  const parsed = parseAbilityRecord(input);
  input.future.enabled = false;
  input.attacks[0]!.dmg.futureDamage = true;

  assert.deepEqual(parsed.future, { enabled: true });
  assert.equal(parsed.targetSelect.futureTarget, 'kept');
  assert.deepEqual(parsed.depiction.futureDepiction, [1, 2]);
  assert.equal(parsed.attacks?.[0]?.futureAttack, 4);
  assert.equal(parsed.attacks?.[0]?.dmg?.futureDamage, false);
  assert.equal(parsed.attacks?.[0]?.save?.futureSave, 'kept');
  assert.deepEqual(parsed.statuses?.[0]?.futureStatus, { value: 3 });
  assert.equal(parsed.restores?.[0]?.futureRestore, true);
  assert.equal(parsed.damages?.[0]?.futureDirectDamage, null);
});

test('accepts the legacy projectile animation representation losslessly', () => {
  const input = createDefaultAbilityRecord('LEGACY');
  delete input.depiction.projectileType;
  input.depiction.projectileAnim = 'arrow_normal_e';
  const parsed = parseAbilityRecord(input);
  assert.equal(parsed.depiction.projectileType, undefined);
  assert.equal(parsed.depiction.projectileAnim, 'arrow_normal_e');
});

test('reports precise nested validation paths', () => {
  const invalid = createDefaultAbilityRecord('BAD');
  invalid.attacks = [
    {
      attackClass: 'ATTACK_CLASS_MELEE',
      dmg: { ...createDefaultAttackDamage(), dmgDice: ['D6'] },
    },
  ];
  (invalid.attacks[0]!.dmg!.dmgDice as string[])[0] = 'D3';

  assert.throws(
    () => parseAbilityRecord(invalid),
    (error: unknown) => {
      assert.ok(error instanceof AbilityParseError);
      assert.equal(error.path, 'ability.attacks[0].dmg.dmgDice[0]');
      return true;
    },
  );
});

test('rejects duplicate names like the C++ collection loader', () => {
  assert.throws(
    () =>
      parseAbilityCollection([
        createDefaultAbilityRecord('SAME'),
        createDefaultAbilityRecord('SAME'),
      ]),
    /abilities\[1\]\.name: duplicate ability "SAME"/,
  );
});

test('clones deeply with deterministic collision-free names', () => {
  assert.equal(
    createUniqueAbilityName('FIRE', ['FIRE_copy', 'FIRE_copy2']),
    'FIRE_copy3',
  );
  const source = parseAbilityRecord({
    ...createDefaultAbilityRecord('FIRE'),
    future: { version: 1 },
    attacks: [
      { attackClass: 'ATTACK_CLASS_MELEE', dmg: createDefaultAttackDamage() },
    ],
  });
  const clone = cloneAbilityRecord(source, ['FIRE', 'FIRE_copy']);
  (clone.future as { version: number }).version = 2;
  clone.attacks![0]!.dmg!.dmgDice[0] = 'D12';
  assert.equal(clone.name, 'FIRE_copy2');
  assert.deepEqual(source.future, { version: 1 });
  assert.equal(source.attacks?.[0]?.dmg?.dmgDice[0], 'D6');
});
