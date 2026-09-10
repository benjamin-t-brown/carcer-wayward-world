import type { JsonObject } from '../../database/types.js';
import type {
  EventAudioInfo,
  EventChoice,
  EventChoiceAlternateText,
  EventNode,
  EventSwitchCase,
  EventVariable,
  SpecialEventRecord,
} from './types.js';

export class SpecialEventParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'SpecialEventParseError';
  }
}

function fail(path: string, detail: string): never {
  throw new SpecialEventParseError(path, detail);
}

function asObject(value: unknown, path: string): JsonObject {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return fail(path, 'expected an object');
  }
  return value as JsonObject;
}

function requireString(object: JsonObject, key: string, path: string): string {
  const value = object[key];
  if (typeof value !== 'string') {
    return fail(`${path}.${key}`, 'expected a string');
  }
  return value;
}

function optionalString(object: JsonObject, key: string, path: string): void {
  if (object[key] !== undefined) {
    requireString(object, key, path);
  }
}

function optionalBoolean(object: JsonObject, key: string, path: string): void {
  const value = object[key];
  if (value !== undefined && typeof value !== 'boolean') {
    fail(`${path}.${key}`, 'expected a boolean');
  }
}

function optionalNumber(object: JsonObject, key: string, path: string): void {
  const value = object[key];
  if (
    value !== undefined &&
    (typeof value !== 'number' || !Number.isFinite(value))
  ) {
    fail(`${path}.${key}`, 'expected a finite number');
  }
}

function optionalInteger(object: JsonObject, key: string, path: string): void {
  const value = object[key];
  if (value !== undefined && !Number.isSafeInteger(value)) {
    fail(`${path}.${key}`, 'expected a safe integer');
  }
}

function optionalObject(
  object: JsonObject,
  key: string,
  path: string,
): JsonObject | undefined {
  const value = object[key];
  return value === undefined ? undefined : asObject(value, `${path}.${key}`);
}

function validateAudioInfo(
  value: JsonObject,
  path: string,
): asserts value is EventAudioInfo {
  optionalString(value, 'audioName', path);
  optionalInteger(value, 'volume', path);
  optionalInteger(value, 'offset', path);
}

function validateChoiceAlternate(
  value: JsonObject,
  path: string,
): asserts value is EventChoiceAlternateText {
  optionalString(value, 'conditionStr', path);
  optionalString(value, 'text', path);
}

function validateChoice(
  value: JsonObject,
  path: string,
): asserts value is EventChoice {
  for (const key of ['text', 'prefixText', 'conditionStr', 'evalStr', 'next']) {
    optionalString(value, key, path);
  }
  validateOptionalObjectArray(
    value,
    'switchText',
    path,
    validateChoiceAlternate,
  );
}

function validateSwitchCase(
  value: JsonObject,
  path: string,
): asserts value is EventSwitchCase {
  optionalString(value, 'conditionStr', path);
  optionalString(value, 'next', path);
}

function validateVariable(
  value: JsonObject,
  path: string,
): asserts value is EventVariable {
  for (const key of ['id', 'key', 'value', 'importFrom']) {
    optionalString(value, key, path);
  }
}

export function parseEventVariable(
  value: unknown,
  path = 'eventVariable',
): EventVariable {
  const variable = asObject(value, path);
  validateVariable(variable, path);
  return structuredClone(variable) as EventVariable;
}

export function parseEventVariables(
  value: unknown,
  path = 'eventVariables',
): EventVariable[] {
  if (!Array.isArray(value)) fail(path, 'expected an array');
  return value.map((variable, index) =>
    parseEventVariable(variable, `${path}[${index}]`),
  );
}

function validateOptionalObjectArray<T extends JsonObject>(
  object: JsonObject,
  key: string,
  path: string,
  validate: (value: JsonObject, itemPath: string) => asserts value is T,
): void {
  const value = object[key];
  if (value === undefined) return;
  if (!Array.isArray(value)) {
    fail(`${path}.${key}`, 'expected an array');
  }
  value.forEach((item, index) => {
    const itemPath = `${path}.${key}[${index}]`;
    validate(asObject(item, itemPath), itemPath);
  });
}

function validateEventNodeObject(
  node: JsonObject,
  path: string,
): asserts node is EventNode {
  requireString(node, 'id', path);
  const type = requireString(node, 'eventChildType', path);
  for (const key of ['x', 'y', 'h']) optionalNumber(node, key, path);

  if (type === 'EXEC') {
    for (const key of ['p', 'execStr', 'next']) optionalString(node, key, path);
    optionalBoolean(node, 'autoAdvance', path);
    const audio = optionalObject(node, 'audioInfo', path);
    if (audio) validateAudioInfo(audio, `${path}.audioInfo`);
  } else if (type === 'CHOICE') {
    optionalString(node, 'text', path);
    validateOptionalObjectArray(node, 'choices', path, validateChoice);
    const audio = optionalObject(node, 'audioInfo', path);
    if (audio) validateAudioInfo(audio, `${path}.audioInfo`);
  } else if (type === 'SWITCH') {
    optionalString(node, 'defaultNext', path);
    validateOptionalObjectArray(node, 'cases', path, validateSwitchCase);
  } else if (type === 'END') {
    optionalString(node, 'next', path);
  } else if (type === 'COMMENT') {
    optionalString(node, 'comment', path);
  } else if (type === 'KEYWORD') {
    optionalObject(node, 'keywords', path);
  }
}

/**
 * Validate the fields understood by the game/editor while preserving the exact
 * JSON object shape, unknown keys, unknown node kinds, and array ordering.
 */
export function parseEventNode(value: unknown, path = 'eventNode'): EventNode {
  const node = asObject(value, path);
  validateEventNodeObject(node, path);
  return structuredClone(node) as EventNode;
}

export function parseSpecialEventRecord(
  value: unknown,
  path = 'specialEvent',
): SpecialEventRecord {
  const event = asObject(value, path);
  for (const key of ['id', 'title', 'eventType', 'icon']) {
    requireString(event, key, path);
  }
  validateOptionalObjectArray(event, 'vars', path, validateVariable);

  const children = event.children;
  if (children !== undefined) {
    if (!Array.isArray(children)) {
      fail(`${path}.children`, 'expected an array');
    }
    children.forEach((child, index) =>
      validateEventNodeObject(
        asObject(child, `${path}.children[${index}]`),
        `${path}.children[${index}]`,
      ),
    );
  }
  return structuredClone(event) as SpecialEventRecord;
}

export function parseSpecialEventCollection(
  value: unknown,
  path = 'specialEvents',
): SpecialEventRecord[] {
  if (!Array.isArray(value)) {
    fail(path, 'expected an array');
  }
  return value.map((event, index) =>
    parseSpecialEventRecord(event, `${path}[${index}]`),
  );
}
