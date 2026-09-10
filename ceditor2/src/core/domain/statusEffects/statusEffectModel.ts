import type { JsonObject } from '../../database/types.js';
import {
  DAMAGE_TYPES,
  STAT_TYPES,
  STATUS_ACTION_TARGET_TYPES,
  STATUS_EFFECT_CONDITIONS,
  STATUS_EVENT_TYPES,
  type DamageType,
  type StatType,
  type StatusActionTargetType,
  type StatusEffectAction,
  type StatusEffectCondition,
  type StatusEffectCurrentStats,
  type StatusEffectDurationScale,
  type StatusEffectEvent,
  type StatusEffectRecord,
  type StatusEffectResistance,
  type StatusEffectStats,
  type StatusEventType,
} from './types.js';

export class StatusEffectParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'StatusEffectParseError';
  }
}

function parseError(path: string, message: string): never {
  throw new StatusEffectParseError(path, message);
}

function asObject(value: unknown, path: string): JsonObject {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return parseError(path, 'expected an object');
  }
  return value as JsonObject;
}

function requireString(object: JsonObject, key: string, path: string): string {
  const value = object[key];
  if (typeof value !== 'string') {
    return parseError(`${path}.${key}`, 'expected a string');
  }
  return value;
}

function requireInteger(object: JsonObject, key: string, path: string): number {
  const value = object[key];
  if (!Number.isInteger(value)) {
    return parseError(`${path}.${key}`, 'expected an integer');
  }
  return value as number;
}

function requireEnum<T extends string>(
  object: JsonObject,
  key: string,
  values: readonly T[],
  path: string,
): T {
  const value = requireString(object, key, path);
  if (!values.includes(value as T)) {
    return parseError(`${path}.${key}`, `unsupported value "${value}"`);
  }
  return value as T;
}

function optionalObject(object: JsonObject, key: string, path: string) {
  const value = object[key];
  return value === undefined ? undefined : asObject(value, `${path}.${key}`);
}

function parseIntegerFields(
  object: JsonObject,
  keys: readonly string[],
  path: string,
): void {
  for (const key of keys) {
    if (object[key] !== undefined) {
      requireInteger(object, key, path);
    }
  }
}

function validateStats(
  value: JsonObject,
  path: string,
): asserts value is StatusEffectStats {
  parseIntegerFields(value, ['STR', 'MND', 'CON', 'AGI', 'LCK'], path);
}

function validateCurrentStats(
  value: JsonObject,
  path: string,
): asserts value is StatusEffectCurrentStats {
  parseIntegerFields(value, ['HP', 'AP', 'MANA', 'AC'], path);
}

function validateDurationScale(
  value: JsonObject,
  path: string,
): asserts value is StatusEffectDurationScale {
  requireEnum<StatType>(value, 'durationStat', STAT_TYPES, path);
  requireInteger(value, 'durationStatMult', path);
}

function validateResistance(
  value: JsonObject,
  path: string,
): asserts value is StatusEffectResistance {
  requireEnum<DamageType>(value, 'attackType', DAMAGE_TYPES, path);
  requireInteger(value, 'mod', path);
}

function validateEvent(
  value: JsonObject,
  path: string,
): asserts value is StatusEffectEvent {
  requireEnum<StatusEventType>(value, 'type', STATUS_EVENT_TYPES, path);
  requireEnum<StatusEffectCondition>(
    value,
    'condition',
    STATUS_EFFECT_CONDITIONS,
    path,
  );
}

function validateAction(
  value: JsonObject,
  path: string,
): asserts value is StatusEffectAction {
  requireEnum<StatusActionTargetType>(
    value,
    'statusActionTargetType',
    STATUS_ACTION_TARGET_TYPES,
    path,
  );
  requireString(value, 'abilityName', path);
  if (!Array.isArray(value.events)) {
    parseError(`${path}.events`, 'expected an array');
  }
  value.events.forEach((event, index) => {
    validateEvent(
      asObject(event, `${path}.events[${index}]`),
      `${path}.events[${index}]`,
    );
  });
}

function validateOptionalObjectArray<T extends JsonObject>(
  object: JsonObject,
  key: string,
  path: string,
  validate: (value: JsonObject, itemPath: string) => asserts value is T,
): void {
  const value = object[key];
  if (value === undefined) {
    return;
  }
  if (!Array.isArray(value)) {
    parseError(`${path}.${key}`, 'expected an array');
  }
  value.forEach((item, index) => {
    const itemPath = `${path}.${key}[${index}]`;
    validate(asObject(item, itemPath), itemPath);
  });
}

/**
 * Validates a database value against the runtime loader contract and returns a
 * detached editable copy. Unknown JSON fields are deliberately left intact.
 */
export function parseStatusEffectRecord(
  value: unknown,
  path = 'statusEffect',
): StatusEffectRecord {
  const object = asObject(value, path);
  requireString(object, 'name', path);
  requireString(object, 'description', path);
  requireInteger(object, 'baseDuration', path);

  for (const legacyField of ['duration', 'events', 'targetInfo']) {
    if (legacyField in object) {
      parseError(`${path}.${legacyField}`, 'legacy field is not supported');
    }
  }

  const durationScale = optionalObject(object, 'durationScale', path);
  if (durationScale) {
    validateDurationScale(durationScale, `${path}.durationScale`);
  }
  const applyBonuses = optionalObject(object, 'applyBonuses', path);
  if (applyBonuses) {
    validateStats(applyBonuses, `${path}.applyBonuses`);
  }
  const applyCurrentStatChange = optionalObject(
    object,
    'applyCurrentStatChange',
    path,
  );
  if (applyCurrentStatChange) {
    validateCurrentStats(
      applyCurrentStatChange,
      `${path}.applyCurrentStatChange`,
    );
  }
  validateOptionalObjectArray(
    object,
    'applyResistances',
    path,
    validateResistance,
  );
  validateOptionalObjectArray(object, 'actions', path, validateAction);

  return structuredClone(object) as StatusEffectRecord;
}

export function parseStatusEffectCollection(
  value: unknown,
): StatusEffectRecord[] {
  if (!Array.isArray(value)) {
    parseError('statusEffects', 'expected an array');
  }
  return value.map((record, index) =>
    parseStatusEffectRecord(record, `statusEffects[${index}]`),
  );
}

export function createDefaultDurationScale(): StatusEffectDurationScale {
  return {
    durationStat: 'STAT_MND',
    durationStatMult: 1,
  };
}

export function createDefaultStatusAction(
  abilityName = '',
): StatusEffectAction {
  return {
    statusActionTargetType: 'STATUS_ACTION_TARGET_SELF',
    abilityName,
    events: [
      {
        type: 'STATUS_EVENT_ON_APPLIED',
        condition: 'CONDITION_ALWAYS',
      },
    ],
  };
}

export function createStatusEffectRecord(name = ''): StatusEffectRecord {
  return {
    name,
    description: '',
    baseDuration: 1,
    applyResistances: [],
    actions: [],
  };
}

export function createUniqueStatusEffectName(
  sourceName: string,
  existingNames: Iterable<string>,
): string {
  const used = new Set(existingNames);
  const baseName = sourceName.trim() || 'STATUS_EFFECT';
  const firstCandidate = `${baseName}_copy`;
  if (!used.has(firstCandidate)) {
    return firstCandidate;
  }
  for (let suffix = 2; ; suffix += 1) {
    const candidate = `${firstCandidate}${suffix}`;
    if (!used.has(candidate)) {
      return candidate;
    }
  }
}

export function cloneStatusEffectRecord(
  source: StatusEffectRecord,
  existingNames: Iterable<string>,
): StatusEffectRecord {
  const clone = structuredClone(source);
  clone.name = createUniqueStatusEffectName(source.name, existingNames);
  return clone;
}
