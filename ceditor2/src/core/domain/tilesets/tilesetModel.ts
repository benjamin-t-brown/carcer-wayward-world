import type { JsonObject } from '../../database/types.js';
import type {
  StoredTileStepSound,
  TileMetadata,
  TilesetRecord,
} from './types.js';

export class TilesetParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'TilesetParseError';
  }
}

function fail(path: string, detail: string): never {
  throw new TilesetParseError(path, detail);
}

function asObject(value: unknown, path: string): JsonObject {
  if (typeof value !== 'object' || value === null || Array.isArray(value)) {
    return fail(path, 'expected an object');
  }
  return value as JsonObject;
}

function optionalInteger(object: JsonObject, key: string, path: string): void {
  if (object[key] !== undefined && !Number.isInteger(object[key])) {
    fail(`${path}.${key}`, 'expected an integer');
  }
}

function optionalString(object: JsonObject, key: string, path: string): void {
  if (object[key] !== undefined && typeof object[key] !== 'string') {
    fail(`${path}.${key}`, 'expected a string');
  }
}

function optionalBoolean(object: JsonObject, key: string, path: string): void {
  if (object[key] !== undefined && typeof object[key] !== 'boolean') {
    fail(`${path}.${key}`, 'expected a boolean');
  }
}

function validateTerrainCorners(value: unknown, path: string): void {
  const corners = asObject(value, path);
  for (const corner of ['nw', 'ne', 'sw', 'se']) {
    if (typeof corners[corner] !== 'string') {
      fail(`${path}.${corner}`, 'expected a string');
    }
  }
}

function validateTile(value: unknown, path: string): void {
  const tile = asObject(value, path);
  if (!Number.isInteger(tile.id)) fail(`${path}.id`, 'expected an integer');
  optionalString(tile, 'description', path);
  if (
    tile.stepSound !== undefined &&
    tile.stepSound !== null &&
    typeof tile.stepSound !== 'number' &&
    typeof tile.stepSound !== 'string'
  ) {
    fail(`${path}.stepSound`, 'expected a number, numeric string, or null');
  }
  for (const key of ['isWalkable', 'isSeeThrough', 'isDoor', 'isContainer']) {
    optionalBoolean(tile, key, path);
  }
  if (tile.tileTerrainBorderMeta !== undefined) {
    validateTerrainCorners(
      tile.tileTerrainBorderMeta,
      `${path}.tileTerrainBorderMeta`,
    );
  }
}

function validateTerrain(value: unknown, path: string): void {
  const terrain = asObject(value, path);
  for (const key of ['primaryTerrain', 'secondaryTerrain']) {
    if (typeof terrain[key] !== 'string') {
      fail(`${path}.${key}`, 'expected a string');
    }
  }
  for (const key of ['mode', 'startTileId']) {
    if (!Number.isInteger(terrain[key])) {
      fail(`${path}.${key}`, 'expected an integer');
    }
  }
}

/** Validate values consumed by the game/editor and return a detached copy. */
export function parseTilesetRecord(
  value: unknown,
  path = 'tileset',
): TilesetRecord {
  const record = asObject(value, path);
  if (typeof record.name !== 'string') {
    fail(`${path}.name`, 'expected a string');
  }
  optionalString(record, 'spriteBase', path);
  for (const key of ['imageWidth', 'imageHeight', 'tileWidth', 'tileHeight']) {
    optionalInteger(record, key, path);
  }
  if (record.tiles !== undefined) {
    if (!Array.isArray(record.tiles))
      fail(`${path}.tiles`, 'expected an array');
    record.tiles.forEach((tile, index) =>
      validateTile(tile, `${path}.tiles[${index}]`),
    );
  }
  if (record.terrain !== undefined) {
    validateTerrain(record.terrain, `${path}.terrain`);
  }
  return structuredClone(record) as TilesetRecord;
}

export function parseTilesetCollection(value: unknown): TilesetRecord[] {
  if (!Array.isArray(value)) fail('tilesets', 'expected an array');
  const records = value.map((record, index) =>
    parseTilesetRecord(record, `tilesets[${index}]`),
  );
  const seen = new Set<string>();
  records.forEach((record, index) => {
    if (seen.has(record.name)) {
      fail(`tilesets[${index}].name`, `duplicate tileset "${record.name}"`);
    }
    seen.add(record.name);
  });
  return records;
}

export function createDefaultTile(id = 0): TileMetadata {
  return {
    id,
    description: '',
    stepSound: 0,
    isWalkable: true,
    isSeeThrough: true,
    isDoor: false,
    isContainer: false,
  };
}

export function createDefaultTileset(name = ''): TilesetRecord {
  return {
    name,
    spriteBase: '',
    imageWidth: 28,
    imageHeight: 32,
    tileWidth: 28,
    tileHeight: 32,
    tiles: [createDefaultTile(0)],
  };
}

export function createUniqueTilesetName(
  sourceName: string,
  existingNames: Iterable<string>,
): string {
  const used = new Set(existingNames);
  const base = sourceName.trim() || 'TILESET';
  const first = `${base}_copy`;
  if (!used.has(first)) return first;
  for (let suffix = 2; ; suffix += 1) {
    const candidate = `${first}${suffix}`;
    if (!used.has(candidate)) return candidate;
  }
}

export function cloneTilesetRecord(
  source: TilesetRecord,
  existingNames: Iterable<string>,
): TilesetRecord {
  const clone = structuredClone(source);
  clone.name = createUniqueTilesetName(source.name, existingNames);
  return clone;
}

export function stepSoundNumber(
  value: StoredTileStepSound | undefined,
): number {
  const parsed = typeof value === 'string' ? Number.parseInt(value, 10) : value;
  return typeof parsed === 'number' && parsed >= 0 && parsed <= 3 ? parsed : 0;
}

/** Resize metadata to the image grid without modifying retained tile objects. */
export function reconcileTilesToDimensions(
  source: TilesetRecord,
): TilesetRecord {
  const imageWidth = source.imageWidth ?? 0;
  const imageHeight = source.imageHeight ?? 0;
  const tileWidth = source.tileWidth ?? 0;
  const tileHeight = source.tileHeight ?? 0;
  const expected =
    tileWidth > 0 && tileHeight > 0
      ? Math.floor(imageWidth / tileWidth) *
        Math.floor(imageHeight / tileHeight)
      : 0;
  const current = source.tiles ?? [];
  const tiles = current.slice(0, expected);
  for (let index = tiles.length; index < expected; index += 1) {
    tiles.push(createDefaultTile(index));
  }
  return { ...source, tiles };
}
