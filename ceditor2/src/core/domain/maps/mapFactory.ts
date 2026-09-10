import type { MapRecord } from './types.js';

export interface BlankMapOptions {
  name: string;
  label?: string;
  width: number;
  height: number;
  type?: 'TOWN' | 'OUTDOOR';
  spriteWidth?: number;
  spriteHeight?: number;
  layers?: readonly number[];
  tilesets?: readonly string[];
}

/** Create the compact JSON shape consumed by the existing C++ map loader. */
export function createBlankMap(options: BlankMapOptions): MapRecord {
  const name = options.name.trim();
  if (!name) throw new Error('A blank map requires a name');
  const width = positiveInteger(options.width, 'width');
  const height = positiveInteger(options.height, 'height');
  const layers = uniqueLayers(options.layers ?? [0]);
  const tilesets = options.tilesets?.length ? [...options.tilesets] : [''];
  if (!tilesets.includes('')) tilesets.unshift('');

  const tiles: Record<string, number[]> = {};
  for (const layer of layers) {
    tiles[String(layer)] = new Array<number>(width * height * 2).fill(0);
  }

  return {
    name,
    label: options.label ?? name,
    type: options.type ?? 'TOWN',
    width,
    height,
    spriteWidth: positiveInteger(options.spriteWidth ?? 28, 'spriteWidth'),
    spriteHeight: positiveInteger(options.spriteHeight ?? 32, 'spriteHeight'),
    tilesets,
    layers,
    tiles,
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };
}

function positiveInteger(value: number, field: string): number {
  if (!Number.isSafeInteger(value) || value <= 0) {
    throw new Error(`${field} must be a positive safe integer`);
  }
  return value;
}

function uniqueLayers(layers: readonly number[]): number[] {
  if (!layers.length) return [0];
  const unique = [...new Set(layers)];
  if (unique.some((layer) => !Number.isSafeInteger(layer))) {
    throw new Error('layers must contain safe integers');
  }
  return unique;
}
