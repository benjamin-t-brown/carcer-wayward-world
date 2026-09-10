import type { JsonObject } from '../../database/types.js';
import {
  ABILITY_COST_TYPES,
  ABILITY_TYPES,
  ATTACK_CLASSES,
  CURRENT_STAT_TYPES,
  DAMAGE_TYPES,
  DICE_TYPES,
  PROJECTILE_PATHS,
  PROJECTILE_TYPES,
  STAT_TYPES,
  TARGET_ALLEGIANCE_TYPES,
  TARGET_SELECT_TYPES,
  type AbilityAttack,
  type AbilityAttackDamage,
  type AbilityDamage,
  type AbilityDepiction,
  type AbilityRecord,
  type AbilityRestore,
  type AbilitySave,
  type AbilityStatus,
  type AbilityTargetSelect,
} from './types.js';

export class AbilityParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'AbilityParseError';
  }
}

function fail(path: string, detail: string): never {
  throw new AbilityParseError(path, detail);
}

function asObject(value: unknown, path: string): JsonObject {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return fail(path, 'expected an object');
  }
  return value as JsonObject;
}

function stringField(value: JsonObject, key: string, path: string): string {
  const field = value[key];
  return typeof field === 'string'
    ? field
    : fail(`${path}.${key}`, 'expected a string');
}

function numberField(value: JsonObject, key: string, path: string): number {
  const field = value[key];
  return typeof field === 'number' && Number.isFinite(field)
    ? field
    : fail(`${path}.${key}`, 'expected a finite number');
}

function integerField(value: JsonObject, key: string, path: string): number {
  const field = value[key];
  return Number.isInteger(field)
    ? (field as number)
    : fail(`${path}.${key}`, 'expected an integer');
}

function enumField<T extends string>(
  value: JsonObject,
  key: string,
  allowed: readonly T[],
  path: string,
): T {
  const field = stringField(value, key, path);
  return allowed.includes(field as T)
    ? (field as T)
    : fail(`${path}.${key}`, `unsupported value "${field}"`);
}

function objectField(value: JsonObject, key: string, path: string): JsonObject {
  return asObject(value[key], `${path}.${key}`);
}

function optionalObject(
  value: JsonObject,
  key: string,
  path: string,
): JsonObject | undefined {
  return value[key] === undefined
    ? undefined
    : asObject(value[key], `${path}.${key}`);
}

function validateDice(value: JsonObject, key: string, path: string): void {
  const dice = value[key];
  if (!Array.isArray(dice)) {
    fail(`${path}.${key}`, 'expected an array');
  }
  dice.forEach((die, index) => {
    if (typeof die !== 'string' || !DICE_TYPES.includes(die as never)) {
      fail(`${path}.${key}[${index}]`, `unsupported die "${String(die)}"`);
    }
  });
}

function validateSave(
  value: JsonObject,
  path: string,
): asserts value is AbilitySave {
  enumField(value, 'saveStat', STAT_TYPES, path);
  integerField(value, 'saveBase', path);
  enumField(value, 'saveAgainst', STAT_TYPES, path);
  integerField(value, 'saveAgainstBase', path);
}

function validateAttackDamage(
  value: JsonObject,
  path: string,
): asserts value is AbilityAttackDamage {
  validateDice(value, 'dmgDice', path);
  integerField(value, 'dmgBonus', path);
  enumField(value, 'dmgStat', STAT_TYPES, path);
  numberField(value, 'dmgStatMult', path);
  integerField(value, 'attackBonus', path);
}

function validateAttack(
  value: JsonObject,
  path: string,
): asserts value is AbilityAttack {
  enumField(value, 'attackClass', ATTACK_CLASSES, path);
  if (value.damageType !== undefined) {
    enumField(value, 'damageType', DAMAGE_TYPES, path);
  }
  const damage = optionalObject(value, 'dmg', path);
  if (damage) validateAttackDamage(damage, `${path}.dmg`);
  const save = optionalObject(value, 'save', path);
  if (save) validateSave(save, `${path}.save`);
}

function validateStatus(
  value: JsonObject,
  path: string,
): asserts value is AbilityStatus {
  stringField(value, 'statusEffect', path);
  const save = optionalObject(value, 'save', path);
  if (save) validateSave(save, `${path}.save`);
  if (value.baseDuration !== undefined)
    integerField(value, 'baseDuration', path);
  if (value.durationBonus !== undefined)
    integerField(value, 'durationBonus', path);
}

function validateRestore(
  value: JsonObject,
  path: string,
): asserts value is AbilityRestore {
  enumField(value, 'restoreWhich', CURRENT_STAT_TYPES, path);
  validateDice(value, 'restoreDice', path);
  integerField(value, 'restoreBonus', path);
  enumField(value, 'restoreStat', STAT_TYPES, path);
  integerField(value, 'restoreStatMult', path);
}

function validateDamage(
  value: JsonObject,
  path: string,
): asserts value is AbilityDamage {
  enumField(value, 'damageType', DAMAGE_TYPES, path);
  validateDice(value, 'dmgDice', path);
  integerField(value, 'dmgBonus', path);
  enumField(value, 'dmgStat', STAT_TYPES, path);
  numberField(value, 'dmgStatMult', path);
}

function validateTarget(
  value: JsonObject,
  path: string,
): asserts value is AbilityTargetSelect {
  enumField(value, 'targetType', TARGET_SELECT_TYPES, path);
  enumField(value, 'allegianceSelectType', TARGET_ALLEGIANCE_TYPES, path);
  integerField(value, 'numTargetableUnits', path);
  const zone = objectField(value, 'zoneSize', path);
  integerField(zone, 'x', `${path}.zoneSize`);
  integerField(zone, 'y', `${path}.zoneSize`);
  integerField(value, 'range', path);
}

function validateDepiction(
  value: JsonObject,
  path: string,
): asserts value is AbilityDepiction {
  stringField(value, 'dmgAnim', path);
  if (value.projectileType !== undefined) {
    enumField(value, 'projectileType', PROJECTILE_TYPES, path);
  } else {
    stringField(value, 'projectileAnim', path);
  }
  enumField(value, 'projectilePath', PROJECTILE_PATHS, path);
  stringField(value, 'startSound', path);
  stringField(value, 'dmgSound', path);
}

function validateArray(
  value: JsonObject,
  key: string,
  path: string,
  validate: (item: JsonObject, itemPath: string) => void,
): void {
  const array = value[key];
  if (array === undefined) return;
  if (!Array.isArray(array)) fail(`${path}.${key}`, 'expected an array');
  array.forEach((item, index) => {
    const itemPath = `${path}.${key}[${index}]`;
    validate(asObject(item, itemPath), itemPath);
  });
}

/** Validate loader-visible fields and return a detached, lossless editable copy. */
export function parseAbilityRecord(
  value: unknown,
  path = 'ability',
): AbilityRecord {
  const record = asObject(value, path);
  stringField(record, 'name', path);
  stringField(record, 'label', path);
  stringField(record, 'description', path);
  stringField(record, 'icon', path);
  enumField(record, 'type', ABILITY_TYPES, path);
  validateTarget(
    objectField(record, 'targetSelect', path),
    `${path}.targetSelect`,
  );
  integerField(record, 'apCost', path);
  enumField(record, 'costType', ABILITY_COST_TYPES, path);
  integerField(record, 'costValue', path);
  validateDepiction(
    objectField(record, 'depiction', path),
    `${path}.depiction`,
  );
  validateArray(record, 'attacks', path, validateAttack);
  validateArray(record, 'statuses', path, validateStatus);
  validateArray(record, 'restores', path, validateRestore);
  validateArray(record, 'damages', path, validateDamage);
  return structuredClone(record) as AbilityRecord;
}

export function parseAbilityCollection(value: unknown): AbilityRecord[] {
  if (!Array.isArray(value)) fail('abilities', 'expected an array');
  const records = value.map((record, index) =>
    parseAbilityRecord(record, `abilities[${index}]`),
  );
  const seen = new Set<string>();
  records.forEach((record, index) => {
    if (seen.has(record.name)) {
      fail(`abilities[${index}].name`, `duplicate ability "${record.name}"`);
    }
    seen.add(record.name);
  });
  return records;
}

export function createDefaultAbilitySave(): AbilitySave {
  return {
    saveStat: 'STAT_MND',
    saveBase: 0,
    saveAgainst: 'STAT_MND',
    saveAgainstBase: 0,
  };
}

export function createDefaultAttackDamage(): AbilityAttackDamage {
  return {
    dmgDice: ['D6'],
    dmgBonus: 0,
    dmgStat: 'STAT_STR',
    dmgStatMult: 1,
    attackBonus: 0,
  };
}

export function createDefaultAbilityRecord(name = ''): AbilityRecord {
  return {
    name,
    label: '',
    description: '',
    icon: '',
    type: 'ABILITY_ATTACK',
    targetSelect: {
      targetType: 'TARGET_UNIT',
      allegianceSelectType: 'TARGET_ALLEGIANCE_OTHER',
      numTargetableUnits: 1,
      zoneSize: { x: 1, y: 1 },
      range: 1,
    },
    apCost: 0,
    costType: 'ABILITY_COST_NONE',
    costValue: 0,
    depiction: {
      dmgAnim: '',
      projectileType: 'PROJECTILE_NONE',
      projectilePath: 'PROJECTILE_PATH_NONE',
      startSound: '',
      dmgSound: '',
    },
    attacks: [],
    statuses: [],
    restores: [],
    damages: [],
  };
}

export function createUniqueAbilityName(
  sourceName: string,
  existingNames: Iterable<string>,
): string {
  const used = new Set(existingNames);
  const base = sourceName.trim() || 'ABILITY';
  const first = `${base}_copy`;
  if (!used.has(first)) return first;
  for (let suffix = 2; ; suffix += 1) {
    const candidate = `${first}${suffix}`;
    if (!used.has(candidate)) return candidate;
  }
}

export function cloneAbilityRecord(
  source: AbilityRecord,
  existingNames: Iterable<string>,
): AbilityRecord {
  const clone = structuredClone(source);
  clone.name = createUniqueAbilityName(source.name, existingNames);
  return clone;
}
