import type { MapRecord } from '../maps/index.js';
import { createBlankMap } from '../maps/index.js';
import type { MapGridRecord } from './types.js';

export interface BlankMapGridOptions {
  name: string;
  label?: string;
  gridWidth: number;
  gridHeight: number;
  mapWidth: number;
  mapHeight: number;
  mapType?: 'TOWN' | 'OUTDOOR';
  spriteWidth?: number;
  spriteHeight?: number;
  layers?: readonly number[];
  tilesets?: readonly string[];
  existingMapNames?: Iterable<string>;
  /** Injectable for deterministic tests. Return an API-safe candidate suffix. */
  nextSuffix?: () => string;
}

export interface BlankMapGridResult {
  grid: MapGridRecord;
  maps: MapRecord[];
}

/**
 * Create a grid and all of its backing map partitions in one detached result.
 * Names are generated once and stored in `cells`; reopening never regenerates
 * them. Saving the returned maps and grid through Save All keeps the operation
 * atomic without introducing a new game data format.
 */
export function createBlankMapGrid(
  options: BlankMapGridOptions,
): BlankMapGridResult {
  const name = options.name.trim();
  if (!name) throw new Error('A map grid requires a name');
  const gridWidth = positiveInteger(options.gridWidth, 'gridWidth');
  const gridHeight = positiveInteger(options.gridHeight, 'gridHeight');
  const mapWidth = positiveInteger(options.mapWidth, 'mapWidth');
  const mapHeight = positiveInteger(options.mapHeight, 'mapHeight');
  const total = gridWidth * gridHeight;
  if (!Number.isSafeInteger(total)) {
    throw new Error('Grid dimensions produce an unsafe partition count');
  }

  const used = new Set(
    [...(options.existingMapNames ?? [])]
      .map((candidate) => candidate.trim())
      .filter(Boolean),
  );
  const prefix = apiPrefix(name);
  const nextSuffix = options.nextSuffix ?? randomSuffix;
  const cells: string[][] = [];
  const maps: MapRecord[] = [];

  for (let cellY = 0; cellY < gridHeight; cellY += 1) {
    const row: string[] = [];
    for (let cellX = 0; cellX < gridWidth; cellX += 1) {
      const mapName = uniqueMapName(prefix, used, nextSuffix);
      used.add(mapName);
      row.push(mapName);
      maps.push(
        createBlankMap({
          name: mapName,
          label: `${options.label?.trim() || name} (${cellX + 1}, ${cellY + 1})`,
          width: mapWidth,
          height: mapHeight,
          type: options.mapType,
          spriteWidth: options.spriteWidth,
          spriteHeight: options.spriteHeight,
          layers: options.layers,
          tilesets: options.tilesets,
        }),
      );
    }
    cells.push(row);
  }

  return {
    grid: {
      name,
      label: options.label ?? name,
      gridWidth,
      gridHeight,
      mapWidth,
      mapHeight,
      cells,
    },
    maps,
  };
}

function uniqueMapName(
  prefix: string,
  used: ReadonlySet<string>,
  nextSuffix: () => string,
): string {
  for (let attempt = 0; attempt < 1000; attempt += 1) {
    const suffix = nextSuffix()
      .toLocaleLowerCase()
      .replace(/[^a-z0-9]/g, '');
    if (!suffix) continue;
    const candidate = `${prefix}_${suffix}`;
    if (!used.has(candidate)) return candidate;
  }
  throw new Error('Could not generate a unique map API name');
}

function apiPrefix(value: string): string {
  const prefix = value
    .normalize('NFKD')
    .toLocaleLowerCase()
    .replace(/[^a-z0-9]+/g, '_')
    .replace(/^_+|_+$/g, '');
  return prefix || 'map';
}

function randomSuffix(): string {
  const bytes = new Uint8Array(8);
  globalThis.crypto.getRandomValues(bytes);
  return [...bytes].map((byte) => byte.toString(36).padStart(2, '0')).join('');
}

function positiveInteger(value: number, field: string): number {
  if (!Number.isSafeInteger(value) || value <= 0) {
    throw new Error(`${field} must be a positive safe integer`);
  }
  return value;
}
