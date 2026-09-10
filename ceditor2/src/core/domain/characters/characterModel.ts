import type { JsonObject } from '../../database/types.js';
import {
  CHARACTER_BEHAVIORS,
  CHARACTER_TYPES,
  COMBAT_BEHAVIORS,
  type CharacterRecord,
  type CharacterStats,
} from './types.js';

export class CharacterParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'CharacterParseError';
  }
}

function fail(path: string, detail: string): never {
  throw new CharacterParseError(path, detail);
}

function asObject(value: unknown, path: string): JsonObject {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return fail(path, 'expected an object');
  }
  return value as JsonObject;
}

function stringField(object: JsonObject, key: string, path: string): string {
  const value = object[key];
  return typeof value === 'string'
    ? value
    : fail(`${path}.${key}`, 'expected a string');
}

function optionalString(object: JsonObject, key: string, path: string): void {
  if (object[key] !== undefined) stringField(object, key, path);
}

function optionalInteger(object: JsonObject, key: string, path: string): void {
  const value = object[key];
  if (value !== undefined && !Number.isSafeInteger(value)) {
    fail(`${path}.${key}`, 'expected a safe integer');
  }
}

function enumField<T extends string>(
  object: JsonObject,
  key: string,
  values: readonly T[],
  path: string,
): T {
  const value = stringField(object, key, path);
  return values.includes(value as T)
    ? (value as T)
    : fail(`${path}.${key}`, `unsupported value "${value}"`);
}

function optionalObject(
  object: JsonObject,
  key: string,
  path: string,
): JsonObject | undefined {
  const value = object[key];
  return value === undefined ? undefined : asObject(value, `${path}.${key}`);
}

function integerFields(
  object: JsonObject,
  keys: readonly string[],
  path: string,
): void {
  keys.forEach((key) => optionalInteger(object, key, path));
}

const GENERIC_STATS = ['str', 'mnd', 'con', 'agi', 'lck'] as const;
const WEAPON_STATS = ['edged', 'pole', 'blunt', 'range', 'unarmed'] as const;
const MAGIC_STATS = [
  'mana',
  'abilityPower',
  'attunement',
  'faith',
  'lore',
] as const;
const BODY_STATS = [
  'resistPhysical',
  'resistMagical',
  'healingEffectiveness',
  'dr',
  'armorTraining',
] as const;
const SKILLS = [
  'trickery',
  'stealth',
  'social',
  'magicItemUse',
  'cooking',
  'acrobatics',
  'survival',
  'focus',
  'conditioning',
] as const;

function validateStats(object: JsonObject, path: string): void {
  const generic = optionalObject(object, 'generic', path);
  if (generic) integerFields(generic, GENERIC_STATS, `${path}.generic`);
  const trainable = optionalObject(object, 'trainable', path);
  if (trainable) {
    const weapon = optionalObject(trainable, 'weapon', `${path}.trainable`);
    if (weapon) integerFields(weapon, WEAPON_STATS, `${path}.trainable.weapon`);
    const magic = optionalObject(trainable, 'magic', `${path}.trainable`);
    if (magic) integerFields(magic, MAGIC_STATS, `${path}.trainable.magic`);
    const body = optionalObject(trainable, 'body', `${path}.trainable`);
    if (body) integerFields(body, BODY_STATS, `${path}.trainable.body`);
  }
  const skills = optionalObject(object, 'skills', path);
  if (skills) integerFields(skills, SKILLS, `${path}.skills`);
}

/** Validate loader-visible fields and return a detached, lossless copy. */
export function parseCharacterRecord(
  value: unknown,
  path = 'character',
): CharacterRecord {
  const record = asObject(value, path);
  enumField(record, 'type', CHARACTER_TYPES, path);
  for (const key of ['name', 'label', 'spritesheet']) {
    stringField(record, key, path);
  }
  const spriteOffset = record.spriteOffset;
  if (typeof spriteOffset !== 'string' && !Number.isSafeInteger(spriteOffset)) {
    fail(`${path}.spriteOffset`, 'expected a string or safe integer');
  }

  const stats = optionalObject(record, 'stats', path);
  if (stats) validateStats(stats, `${path}.stats`);

  const talk = optionalObject(record, 'talk', path);
  if (talk) {
    optionalString(talk, 'talkName', `${path}.talk`);
    optionalString(talk, 'portraitName', `${path}.talk`);
  }
  const behavior = optionalObject(record, 'behavior', path);
  if (behavior?.behaviorName !== undefined) {
    const name = stringField(behavior, 'behaviorName', `${path}.behavior`);
    if (name && !CHARACTER_BEHAVIORS.includes(name as never)) {
      fail(`${path}.behavior.behaviorName`, `unsupported value "${name}"`);
    }
  }
  const combat = optionalObject(record, 'combat', path);
  if (combat) {
    integerFields(combat, ['hp', 'mp'], `${path}.combat`);
    optionalString(combat, 'dropTable', `${path}.combat`);
    const legacyStats = optionalObject(combat, 'stats', `${path}.combat`);
    if (legacyStats)
      integerFields(legacyStats, GENERIC_STATS, `${path}.combat.stats`);
  }
  const combatBehavior = optionalObject(record, 'combatBehavior', path);
  if (combatBehavior) {
    for (const key of ['town', 'combat']) {
      if (combatBehavior[key] !== undefined) {
        enumField(
          combatBehavior,
          key,
          COMBAT_BEHAVIORS,
          `${path}.combatBehavior`,
        );
      }
    }
  }
  const sound = optionalObject(record, 'sound', path);
  if (sound) {
    for (const key of [
      'deathSoundName',
      'weaponSoundName',
      'deathSound',
      'weaponSound',
    ]) {
      optionalString(sound, key, `${path}.sound`);
    }
  }
  if (record.statuses !== undefined) {
    if (!Array.isArray(record.statuses)) {
      fail(`${path}.statuses`, 'expected an array');
    }
    record.statuses.forEach((value, index) => {
      const status = asObject(value, `${path}.statuses[${index}]`);
      stringField(status, 'status', `${path}.statuses[${index}]`);
    });
  }
  const vision = optionalObject(record, 'vision', path);
  if (vision) optionalInteger(vision, 'radius', `${path}.vision`);

  return structuredClone(record) as CharacterRecord;
}

export function parseCharacterCollection(value: unknown): CharacterRecord[] {
  if (!Array.isArray(value)) fail('characters', 'expected an array');
  const result = value.map((record, index) =>
    parseCharacterRecord(record, `characters[${index}]`),
  );
  const seen = new Set<string>();
  result.forEach((record, index) => {
    if (seen.has(record.name)) {
      fail(`characters[${index}].name`, `duplicate character "${record.name}"`);
    }
    seen.add(record.name);
  });
  return result;
}

export function createDefaultCharacterStats(): CharacterStats {
  return {
    generic: { str: 0, mnd: 0, con: 0, agi: 0, lck: 0 },
    trainable: {
      weapon: { edged: 0, pole: 0, blunt: 0, range: 0, unarmed: 0 },
      magic: { mana: 0, abilityPower: 0, attunement: 0, faith: 0, lore: 0 },
      body: {
        resistPhysical: 0,
        resistMagical: 0,
        healingEffectiveness: 0,
        dr: 0,
        armorTraining: 0,
      },
    },
    skills: {
      trickery: 0,
      stealth: 0,
      social: 0,
      magicItemUse: 0,
      cooking: 0,
      acrobatics: 0,
      survival: 0,
      focus: 0,
      conditioning: 0,
    },
  };
}

export function createDefaultCharacterRecord(name = ''): CharacterRecord {
  return {
    type: 'TOWNSPERSON',
    name,
    label: '',
    spritesheet: 'actors0',
    spriteOffset: 0,
    stats: createDefaultCharacterStats(),
  };
}

export function createUniqueCharacterName(
  sourceName: string,
  existingNames: Iterable<string>,
): string {
  const used = new Set(existingNames);
  const base = sourceName.trim() || 'CHARACTER';
  const first = `${base}_copy`;
  if (!used.has(first)) return first;
  for (let suffix = 2; ; suffix += 1) {
    const candidate = `${first}${suffix}`;
    if (!used.has(candidate)) return candidate;
  }
}

export function cloneCharacterRecord(
  source: CharacterRecord,
  existingNames: Iterable<string>,
): CharacterRecord {
  const clone = structuredClone(source);
  clone.name = createUniqueCharacterName(source.name, existingNames);
  return clone;
}
