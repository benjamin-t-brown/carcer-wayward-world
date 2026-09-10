import type { JsonObject } from '../../database/types.js';
import {
  CURRENT_STAT_TYPES,
  DICE_TYPES,
  STAT_TYPES,
  type AbilityAttackDamage,
  type AbilityRestore,
} from '../abilities/index.js';
import {
  ITEM_USABILITIES,
  ITEM_TYPES,
  RUNE_TYPES,
  type ItemRecord,
  type ItemStatusEffectRef,
  type ItemType,
  type ItemUsability,
  type RuneType,
} from './types.js';

export class ItemParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'ItemParseError';
  }
}

function fail(path: string, detail: string): never {
  throw new ItemParseError(path, detail);
}

function asObject(value: unknown, path: string): JsonObject {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return fail(path, 'expected an object');
  }
  return value as JsonObject;
}

function stringField(value: JsonObject, key: string, path: string): string {
  const fieldValue = value[key];
  return typeof fieldValue === 'string'
    ? fieldValue
    : fail(`${path}.${key}`, 'expected a string');
}

function integerField(value: JsonObject, key: string, path: string): number {
  const fieldValue = value[key];
  return Number.isInteger(fieldValue)
    ? (fieldValue as number)
    : fail(`${path}.${key}`, 'expected an integer');
}

function numberField(value: JsonObject, key: string, path: string): number {
  const fieldValue = value[key];
  return typeof fieldValue === 'number' && Number.isFinite(fieldValue)
    ? fieldValue
    : fail(`${path}.${key}`, 'expected a finite number');
}

function enumField<T extends string>(
  value: JsonObject,
  key: string,
  allowed: readonly T[],
  path: string,
): T {
  const fieldValue = stringField(value, key, path);
  return allowed.includes(fieldValue as T)
    ? (fieldValue as T)
    : fail(`${path}.${key}`, `unsupported value "${fieldValue}"`);
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
  if (!Array.isArray(dice)) fail(`${path}.${key}`, 'expected an array');
  dice.forEach((die, index) => {
    if (typeof die !== 'string' || !DICE_TYPES.includes(die as never)) {
      fail(`${path}.${key}[${index}]`, `unsupported die "${String(die)}"`);
    }
  });
}

function validateDamage(
  value: JsonObject,
  path: string,
): asserts value is AbilityAttackDamage {
  validateDice(value, 'dmgDice', path);
  integerField(value, 'dmgBonus', path);
  enumField(value, 'dmgStat', STAT_TYPES, path);
  numberField(value, 'dmgStatMult', path);
  integerField(value, 'attackBonus', path);
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

function validateObjectArray(
  owner: JsonObject,
  key: string,
  path: string,
  validate: (entry: JsonObject, entryPath: string) => void,
): void {
  const value = owner[key];
  if (value === undefined) return;
  if (!Array.isArray(value)) fail(`${path}.${key}`, 'expected an array');
  value.forEach((entry, index) => {
    const entryPath = `${path}.${key}[${index}]`;
    validate(asObject(entry, entryPath), entryPath);
  });
}

function validateAbilityConfig(
  value: JsonObject,
  path: string,
  allowRestore: boolean,
): void {
  stringField(value, 'abilityName', path);
  validateObjectArray(value, 'dmgOverrides', path, validateDamage);
  if (allowRestore) {
    validateObjectArray(value, 'restoreOverrides', path, validateRestore);
  }
}

function validateStatusEffects(value: JsonObject, path: string): void {
  const effects = value.statusEffects;
  if (effects === undefined) return;
  if (!Array.isArray(effects)) {
    fail(`${path}.statusEffects`, 'expected an array');
  }
  effects.forEach((effect, index) => {
    const effectPath = `${path}.statusEffects[${index}]`;
    if (typeof effect === 'string') return;
    stringField(asObject(effect, effectPath), 'name', effectPath);
  });
}

/** Validate fields consumed by LoadItemTemplates and return a detached copy. */
export function parseItemRecord(value: unknown, path = 'item'): ItemRecord {
  const record = asObject(value, path);
  const itemType = enumField(record, 'itemType', ITEM_TYPES, path);
  stringField(record, 'name', path);
  stringField(record, 'label', path);
  stringField(record, 'icon', path);
  stringField(record, 'description', path);
  integerField(record, 'weight', path);
  integerField(record, 'value', path);

  for (const key of ['stackable', 'indestructable'] as const) {
    if (record[key] !== undefined && typeof record[key] !== 'boolean') {
      fail(`${path}.${key}`, 'expected a boolean');
    }
  }
  if (
    record.itemUsability !== undefined &&
    typeof record.itemUsability !== 'string'
  ) {
    fail(`${path}.itemUsability`, 'expected a string');
  }
  if (
    record.useSpecialEvent !== undefined &&
    typeof record.useSpecialEvent !== 'string'
  ) {
    fail(`${path}.useSpecialEvent`, 'expected a string');
  }

  validateStatusEffects(record, path);
  const weapon = optionalObject(record, 'weapon', path);
  if (weapon) {
    validateAbilityConfig(weapon, `${path}.weapon`, false);
    if (weapon.dmg !== undefined) {
      validateDamage(
        asObject(weapon.dmg, `${path}.weapon.dmg`),
        `${path}.weapon.dmg`,
      );
    }
    if (weapon.attackIndex !== undefined) {
      integerField(weapon, 'attackIndex', `${path}.weapon`);
    }
  }
  const useAbility = optionalObject(record, 'useAbility', path);
  if (useAbility) {
    validateAbilityConfig(useAbility, `${path}.useAbility`, true);
  }

  if (itemType === 'RUNE') {
    enumField(record, 'runeType', RUNE_TYPES, path);
  } else if (record.runeType !== undefined) {
    fail(`${path}.runeType`, 'non-RUNE items must not set runeType');
  }

  return structuredClone(record) as ItemRecord;
}

export function parseItemCollection(value: unknown): ItemRecord[] {
  if (!Array.isArray(value)) fail('items', 'expected an array');
  const records = value.map((record, index) =>
    parseItemRecord(record, `items[${index}]`),
  );
  const seen = new Set<string>();
  records.forEach((record, index) => {
    if (seen.has(record.name)) {
      fail(`items[${index}].name`, `duplicate item "${record.name}"`);
    }
    seen.add(record.name);
  });
  return records;
}

export function itemStatusEffectName(reference: ItemStatusEffectRef): string {
  return typeof reference === 'string' ? reference : reference.name;
}

export function isWeaponItemType(itemType: string): boolean {
  return itemType.startsWith('WEAPON_');
}

export function isItemUsable(itemUsability?: string): boolean {
  return Boolean(
    itemUsability &&
    itemUsability !== 'NOT_USABLE' &&
    ITEM_USABILITIES.includes(itemUsability as never),
  );
}

export function runeIcon(runeType: RuneType): string {
  return `runes_${RUNE_TYPES.indexOf(runeType)}`;
}

export function createDefaultItemRecord(name = ''): ItemRecord {
  return {
    itemType: 'UTILITY',
    name,
    label: '',
    icon: 'ui_item_icons_0',
    description: '',
    weight: 1,
    value: 1,
    stackable: false,
    indestructable: false,
    itemUsability: 'NOT_USABLE',
    statusEffects: [],
  };
}

export function createDefaultDamageOverride(): AbilityAttackDamage {
  return {
    dmgDice: ['D6'],
    dmgBonus: 0,
    dmgStat: 'STAT_STR',
    dmgStatMult: 1,
    attackBonus: 0,
  };
}

export function createDefaultRestoreOverride(): AbilityRestore {
  return {
    restoreWhich: 'CURRENT_STAT_HP',
    restoreDice: ['D6'],
    restoreBonus: 0,
    restoreStat: 'STAT_MND',
    restoreStatMult: 0,
  };
}

export function changeItemType(
  source: ItemRecord,
  itemType: ItemType,
): ItemRecord {
  const next = structuredClone(source);
  next.itemType = itemType;
  if (isWeaponItemType(itemType)) {
    next.weapon ??= { abilityName: '' };
  } else {
    delete next.weapon;
  }
  if (itemType === 'RUNE') {
    next.runeType = RUNE_TYPES.includes(next.runeType as RuneType)
      ? (next.runeType as RuneType)
      : 'HEAT';
    if (!next.icon || /^runes_[0-7]$/.test(next.icon)) {
      next.icon = runeIcon(next.runeType);
    }
  } else {
    delete next.runeType;
  }
  return next;
}

export function changeItemUsability(
  source: ItemRecord,
  itemUsability: ItemUsability,
): ItemRecord {
  const next = structuredClone(source);
  next.itemUsability = itemUsability;
  if (!isItemUsable(itemUsability)) {
    delete next.useAbility;
    delete next.useSpecialEvent;
  } else if (!next.useAbility && !next.useSpecialEvent) {
    next.useAbility = { abilityName: '' };
  }
  return next;
}

export function createUniqueItemName(
  sourceName: string,
  existingNames: Iterable<string>,
): string {
  const used = new Set(existingNames);
  const base = sourceName.trim() || 'ITEM';
  const first = `${base}_copy`;
  if (!used.has(first)) return first;
  for (let suffix = 2; ; suffix += 1) {
    const candidate = `${first}${suffix}`;
    if (!used.has(candidate)) return candidate;
  }
}

export function cloneItemRecord(
  source: ItemRecord,
  existingNames: Iterable<string>,
): ItemRecord {
  const clone = structuredClone(source);
  clone.name = createUniqueItemName(source.name, existingNames);
  return clone;
}
