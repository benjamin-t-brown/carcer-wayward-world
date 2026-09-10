import type { JsonObject } from '../../database/types.js';
import {
  MAP_TYPES,
  type MapCharacterPlacement,
  type MapEventTriggerPlacement,
  type MapItemPlacement,
  type MapLightSource,
  type MapLightSourcePlacement,
  type MapMarkerPlacement,
  type MapRecord,
  type MapTileOverridePlacement,
  type MapTileOverrides,
  type MapTravelTriggerPlacement,
  type MapType,
} from './types.js';

export class MapParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'MapParseError';
  }
}

function fail(path: string, message: string): never {
  throw new MapParseError(path, message);
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

function requireInteger(object: JsonObject, key: string, path: string): number {
  const value = object[key];
  if (!Number.isSafeInteger(value)) {
    return fail(`${path}.${key}`, 'expected a safe integer');
  }
  return value as number;
}

function optionalString(object: JsonObject, key: string, path: string): void {
  if (object[key] !== undefined) {
    requireString(object, key, path);
  }
}

function optionalInteger(object: JsonObject, key: string, path: string): void {
  if (object[key] !== undefined) {
    requireInteger(object, key, path);
  }
}

function optionalPositiveInteger(
  object: JsonObject,
  key: string,
  path: string,
): void {
  if (object[key] === undefined) return;
  if (requireInteger(object, key, path) <= 0) {
    fail(`${path}.${key}`, 'expected a positive integer');
  }
}

function optionalBoolean(object: JsonObject, key: string, path: string): void {
  const value = object[key];
  if (value !== undefined && typeof value !== 'boolean') {
    fail(`${path}.${key}`, 'expected a boolean');
  }
}

function validateTileReference(object: JsonObject, path: string): void {
  optionalInteger(object, 'l', path);
  optionalInteger(object, 'i', path);
}

function validateNamedPlacement(
  object: JsonObject,
  path: string,
): asserts object is
  MapCharacterPlacement | MapItemPlacement | MapMarkerPlacement {
  validateTileReference(object, path);
  optionalString(object, 'name', path);
}

function validateLightSource(
  object: JsonObject,
  path: string,
): asserts object is MapLightSource {
  optionalInteger(object, 'angle', path);
  optionalInteger(object, 'intensity', path);
  optionalInteger(object, 'radius', path);
}

function validateEventTrigger(
  object: JsonObject,
  path: string,
): asserts object is MapEventTriggerPlacement {
  validateTileReference(object, path);
  optionalString(object, 'eventId', path);
  optionalBoolean(object, 'requiresNonCombat', path);
  optionalBoolean(object, 'requiresLook', path);
  optionalString(object, 'overlayVisibility', path);
}

function validateTravelTrigger(
  object: JsonObject,
  path: string,
): asserts object is MapTravelTriggerPlacement {
  validateTileReference(object, path);
  optionalString(object, 'destinationMapName', path);
  optionalString(object, 'destinationMarkerName', path);
  optionalInteger(object, 'destinationX', path);
  optionalInteger(object, 'destinationY', path);
  optionalInteger(object, 'destinationLayer', path);
  optionalBoolean(object, 'requiresAction', path);
  optionalString(object, 'overlayVisibility', path);
}

function validateTileOverrides(
  object: JsonObject,
  path: string,
): asserts object is MapTileOverrides {
  optionalBoolean(object, 'isWalkableOverride', path);
  optionalBoolean(object, 'isSeeThroughOverride', path);
  optionalBoolean(object, 'isContainerOverride', path);
  if (object.lightSourceOverride !== undefined) {
    validateLightSource(
      asObject(object.lightSourceOverride, `${path}.lightSourceOverride`),
      `${path}.lightSourceOverride`,
    );
  }
}

function validateTileOverridePlacement(
  object: JsonObject,
  path: string,
): asserts object is MapTileOverridePlacement {
  validateTileReference(object, path);
  if (object.overrides !== undefined) {
    validateTileOverrides(
      asObject(object.overrides, `${path}.overrides`),
      `${path}.overrides`,
    );
  }
}

function validateLightSourcePlacement(
  object: JsonObject,
  path: string,
): asserts object is MapLightSourcePlacement {
  validateTileReference(object, path);
  validateLightSource(object, path);
}

function validatePlacementArray<T extends JsonObject>(
  map: JsonObject,
  key: string,
  path: string,
  validate: (object: JsonObject, itemPath: string) => asserts object is T,
): void {
  const value = map[key];
  if (value === undefined) {
    return;
  }
  if (!Array.isArray(value)) {
    fail(`${path}.${key}`, 'expected an array');
  }
  value.forEach((entry, index) => {
    const itemPath = `${path}.${key}[${index}]`;
    validate(asObject(entry, itemPath), itemPath);
  });
}

function validateDenseLayers(
  map: JsonObject,
  width: number,
  height: number,
  layers: number[],
  path: string,
): void {
  const tiles = asObject(map.tiles, `${path}.tiles`);
  const expectedLength = width * height * 2;
  if (!Number.isSafeInteger(expectedLength)) {
    fail(`${path}.tiles`, 'map dimensions produce an unsafe cell count');
  }

  for (const layer of layers) {
    if (!Object.hasOwn(tiles, String(layer))) {
      fail(
        `${path}.tiles[${JSON.stringify(String(layer))}]`,
        'missing declared layer',
      );
    }
  }

  for (const [layerKey, graphics] of Object.entries(tiles)) {
    const layerPath = `${path}.tiles[${JSON.stringify(layerKey)}]`;
    if (!/^-?\d+$/.test(layerKey) || !Number.isSafeInteger(Number(layerKey))) {
      fail(layerPath, 'layer key must be a safe integer string');
    }
    if (!Array.isArray(graphics)) {
      fail(layerPath, 'expected a dense graphic array');
    }
    if (graphics.length !== expectedLength) {
      fail(
        layerPath,
        `expected ${expectedLength} entries, received ${graphics.length}`,
      );
    }
    graphics.forEach((entry, index) => {
      if (!Number.isSafeInteger(entry)) {
        fail(`${layerPath}[${index}]`, 'expected a safe integer');
      }
    });
  }
}

/**
 * Parses the current compact editor shape without applying loader defaults or
 * rearranging data. The result is a detached copy with unknown fields intact.
 */
export function parseMapRecord(value: unknown, path = 'map'): MapRecord {
  const map = asObject(value, path);
  requireString(map, 'name', path);

  optionalString(map, 'label', path);
  if (map.type !== undefined) {
    const type = requireString(map, 'type', path);
    if (!MAP_TYPES.includes(type as MapType)) {
      fail(`${path}.type`, `unsupported value "${type}"`);
    }
  }

  const width = requireInteger(map, 'width', path);
  const height = requireInteger(map, 'height', path);
  if (width <= 0) {
    fail(`${path}.width`, 'expected a positive integer');
  }
  if (height <= 0) {
    fail(`${path}.height`, 'expected a positive integer');
  }
  optionalPositiveInteger(map, 'spriteWidth', path);
  optionalPositiveInteger(map, 'spriteHeight', path);

  if (!Array.isArray(map.tilesets)) {
    fail(`${path}.tilesets`, 'expected an array');
  }
  map.tilesets.forEach((tileset, index) => {
    if (typeof tileset !== 'string') {
      fail(`${path}.tilesets[${index}]`, 'expected a string');
    }
  });

  if (!Array.isArray(map.layers)) {
    fail(`${path}.layers`, 'expected an array');
  }
  const layers: number[] = [];
  const seenLayers = new Set<number>();
  map.layers.forEach((layer, index) => {
    if (!Number.isSafeInteger(layer)) {
      fail(`${path}.layers[${index}]`, 'expected a safe integer');
    }
    const layerNumber = layer as number;
    if (seenLayers.has(layerNumber)) {
      fail(`${path}.layers[${index}]`, `duplicate layer ${layerNumber}`);
    }
    seenLayers.add(layerNumber);
    layers.push(layerNumber);
  });
  validateDenseLayers(map, width, height, layers, path);

  validatePlacementArray(map, 'characters', path, validateNamedPlacement);
  validatePlacementArray(map, 'items', path, (item, itemPath) => {
    validateNamedPlacement(item, itemPath);
    optionalInteger(item, 'quantity', itemPath);
  });
  validatePlacementArray(map, 'markers', path, validateNamedPlacement);
  validatePlacementArray(map, 'eventTriggers', path, validateEventTrigger);
  validatePlacementArray(map, 'travelTriggers', path, validateTravelTrigger);
  validatePlacementArray(
    map,
    'tileOverrides',
    path,
    validateTileOverridePlacement,
  );
  validatePlacementArray(
    map,
    'lightSources',
    path,
    validateLightSourcePlacement,
  );

  return structuredClone(map) as MapRecord;
}

export function parseMapCollection(value: unknown): MapRecord[] {
  if (!Array.isArray(value)) {
    fail('maps', 'expected an array');
  }
  return value.map((map, index) => parseMapRecord(map, `maps[${index}]`));
}
