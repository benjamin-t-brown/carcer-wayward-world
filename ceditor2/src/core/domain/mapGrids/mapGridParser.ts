import type { JsonObject } from '../../database/types.js';
import type { MapGridRecord } from './types.js';

export class MapGridParseError extends Error {
  constructor(
    readonly path: string,
    readonly detail: string,
  ) {
    super(`${path}: ${detail}`);
    this.name = 'MapGridParseError';
  }
}

function fail(path: string, detail: string): never {
  throw new MapGridParseError(path, detail);
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

function requirePositiveInteger(
  object: JsonObject,
  key: string,
  path: string,
): number {
  const value = object[key];
  if (!Number.isSafeInteger(value) || Number(value) <= 0) {
    return fail(`${path}.${key}`, 'expected a positive safe integer');
  }
  return value as number;
}

/**
 * Validate a grid without normalizing its dimensions, cells, or unknown data.
 * The returned record is detached from the caller so snapshots stay lossless.
 */
export function parseMapGridRecord(
  value: unknown,
  path = 'mapGrid',
): MapGridRecord {
  const grid = asObject(value, path);
  requireString(grid, 'name', path);

  if (grid.label !== undefined) {
    requireString(grid, 'label', path);
  }

  const gridWidth = requirePositiveInteger(grid, 'gridWidth', path);
  const gridHeight = requirePositiveInteger(grid, 'gridHeight', path);
  requirePositiveInteger(grid, 'mapWidth', path);
  requirePositiveInteger(grid, 'mapHeight', path);

  if (!Array.isArray(grid.cells)) {
    fail(`${path}.cells`, 'expected an array');
  }
  if (grid.cells.length !== gridHeight) {
    fail(
      `${path}.cells`,
      `expected ${gridHeight} rows, received ${grid.cells.length}`,
    );
  }

  grid.cells.forEach((row, cellY) => {
    const rowPath = `${path}.cells[${cellY}]`;
    if (!Array.isArray(row)) {
      fail(rowPath, 'expected an array');
    }
    if (row.length !== gridWidth) {
      fail(rowPath, `expected ${gridWidth} cells, received ${row.length}`);
    }
    row.forEach((cell, cellX) => {
      if (typeof cell !== 'string') {
        fail(`${rowPath}[${cellX}]`, 'expected a string');
      }
    });
  });

  return structuredClone(grid) as MapGridRecord;
}

export function parseMapGridCollection(
  value: unknown,
  path = 'mapGrids',
): MapGridRecord[] {
  if (!Array.isArray(value)) {
    fail(path, 'expected an array');
  }
  return value.map((grid, index) =>
    parseMapGridRecord(grid, `${path}[${index}]`),
  );
}
