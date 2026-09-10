import type { JsonObject } from '../../database/types.js';
import {
  RUNE_TYPES,
  type RuneType,
  type SpellRecord,
  type SpellRuneRequirement,
} from './types.js';

export class SpellParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'SpellParseError';
  }
}

function fail(path: string, detail: string): never {
  throw new SpellParseError(path, detail);
}

function asObject(value: unknown, path: string): JsonObject {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return fail(path, 'expected an object');
  }
  return value as JsonObject;
}

function requireString(object: JsonObject, key: string, path: string): string {
  const value = object[key];
  return typeof value === 'string'
    ? value
    : fail(`${path}.${key}`, 'expected a string');
}

function parseRuneRequirement(
  value: unknown,
  path: string,
): SpellRuneRequirement {
  const object = asObject(value, path);
  const type = requireString(object, 'type', path);
  if (!RUNE_TYPES.includes(type as RuneType)) {
    fail(`${path}.type`, `unsupported value "${type}"`);
  }
  const count = object.count;
  if (!Number.isInteger(count)) {
    fail(`${path}.count`, 'expected an integer');
  }
  if ((count as number) <= 0) {
    fail(`${path}.count`, 'expected a positive integer');
  }
  return object as SpellRuneRequirement;
}

/** Validate loader-visible fields and return a detached, lossless editable copy. */
export function parseSpellRecord(value: unknown, path = 'spell'): SpellRecord {
  const record = asObject(value, path);
  requireString(record, 'name', path);
  requireString(record, 'label', path);
  requireString(record, 'description', path);
  requireString(record, 'icon', path);
  requireString(record, 'abilityName', path);

  if (!Array.isArray(record.requiredRunes)) {
    fail(`${path}.requiredRunes`, 'expected an array');
  }

  const seen = new Set<RuneType>();
  record.requiredRunes.forEach((value, index) => {
    const itemPath = `${path}.requiredRunes[${index}]`;
    const requirement = parseRuneRequirement(value, itemPath);
    if (seen.has(requirement.type)) {
      fail(`${itemPath}.type`, `duplicate rune type "${requirement.type}"`);
    }
    seen.add(requirement.type);
  });

  return structuredClone(record) as SpellRecord;
}

export function parseSpellCollection(value: unknown): SpellRecord[] {
  if (!Array.isArray(value)) {
    fail('spells', 'expected an array');
  }

  const names = new Set<string>();
  return value.map((record, index) => {
    const parsed = parseSpellRecord(record, `spells[${index}]`);
    if (names.has(parsed.name)) {
      fail(`spells[${index}].name`, `duplicate spell name "${parsed.name}"`);
    }
    names.add(parsed.name);
    return parsed;
  });
}

export function createSpellRecord(name = ''): SpellRecord {
  return {
    name,
    label: '',
    description: '',
    icon: 'runes_0',
    abilityName: '',
    requiredRunes: [],
  };
}

export function createUniqueSpellName(
  sourceName: string,
  existingNames: Iterable<string>,
): string {
  const used = new Set(existingNames);
  const baseName = sourceName.trim() || 'SPELL';
  const firstCandidate = `${baseName}_copy`;
  if (!used.has(firstCandidate)) return firstCandidate;
  for (let suffix = 2; ; suffix += 1) {
    const candidate = `${firstCandidate}${suffix}`;
    if (!used.has(candidate)) return candidate;
  }
}

export function cloneSpellRecord(
  source: SpellRecord,
  existingNames: Iterable<string>,
): SpellRecord {
  const clone = structuredClone(source);
  clone.name = createUniqueSpellName(source.name, existingNames);
  return clone;
}
