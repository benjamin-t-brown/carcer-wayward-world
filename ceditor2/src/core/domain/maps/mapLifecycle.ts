import { parseMapGridCollection } from '../mapGrids/mapGridParser.js';
import type { MapGridRecord } from '../mapGrids/types.js';
import { createBlankMap, type BlankMapOptions } from './mapFactory.js';
import { parseMapCollection, parseMapRecord } from './mapParser.js';
import {
  MAP_PLACEMENT_KEYS,
  MAP_TYPES,
  type MapCellGraphic,
  type MapPlacementKey,
  type MapRecord,
  type MapTileReference,
  type MapType,
} from './types.js';

const MAX_ARRAY_LENGTH = 0xffff_ffff;

export class MapLifecycleError extends Error {
  constructor(message: string) {
    super(message);
    this.name = 'MapLifecycleError';
  }
}

export interface CloneMapOptions {
  name: string;
  /** Omit to retain the source label exactly, including its absence. */
  label?: string;
}

export interface MapMetadataPatch {
  /** `null` removes an optional field so the C++ loader applies its default. */
  label?: string | null;
  type?: MapType | null;
  spriteWidth?: number | null;
  spriteHeight?: number | null;
}

export interface ResizeMapOptions {
  width: number;
  height: number;
  /** Graphic used for newly exposed cells. Defaults to the blank pair `[0, 0]`. */
  fill?: MapCellGraphic;
}

export type PlacementDropCounts = Record<MapPlacementKey, number>;

export interface ResizeMapResult {
  map: MapRecord;
  droppedPlacements: PlacementDropCounts;
}

export interface DeleteMapLayerResult {
  map: MapRecord;
  droppedPlacements: PlacementDropCounts;
}

export interface MapGridCellReference {
  gridIndex: number;
  gridName: string;
  cellX: number;
  cellY: number;
}

export interface MapTravelReference {
  mapIndex: number;
  mapName: string;
  triggerIndex: number;
}

export interface MapReferencePreview {
  targetMapName: string;
  mapIndex: number;
  gridCells: readonly MapGridCellReference[];
  travelTriggers: readonly MapTravelReference[];
  /** A same-named grid wins during C++ destination resolution. */
  travelDestinationResolvesToGrid: boolean;
}

export interface MapDatabaseRecords {
  maps: readonly unknown[];
  mapGrids: readonly unknown[];
}

export interface MapDatabaseLifecycleResult {
  maps: MapRecord[];
  mapGrids: MapGridRecord[];
  preview: MapReferencePreview;
}

/** Clone a map without normalizing optional or unknown fields. */
export function cloneMapRecord(
  source: unknown,
  options: CloneMapOptions,
): MapRecord {
  const map = parseMapRecord(source);
  map.name = requireApiName(options.name, 'name');
  if (Object.hasOwn(options, 'label')) {
    map.label = requireString(options.label, 'label');
  }
  return map;
}

/** Rename only the map record. Use `renameMapAcrossDatabase` for persisted data. */
export function renameMapRecord(source: unknown, name: string): MapRecord {
  return cloneMapRecord(source, { name });
}

/** Apply descriptive/runtime metadata without touching identity or dimensions. */
export function updateMapMetadata(
  source: unknown,
  patch: MapMetadataPatch,
): MapRecord {
  const map = parseMapRecord(source);
  applyOptionalString(map, patch, 'label');

  if (Object.hasOwn(patch, 'type')) {
    if (patch.type === null) {
      delete map.type;
    } else if (MAP_TYPES.includes(patch.type as MapType)) {
      map.type = patch.type;
    } else {
      throw new MapLifecycleError('type must be TOWN, OUTDOOR, or null');
    }
  }

  applyOptionalPositiveInteger(map, patch, 'spriteWidth');
  applyOptionalPositiveInteger(map, patch, 'spriteHeight');
  return map;
}

/** Create and append a uniquely named blank map to a detached collection. */
export function createMapInCollection(
  mapsValue: unknown,
  options: BlankMapOptions,
  mapGridsValue: unknown = [],
): MapRecord[] {
  const maps = parseMapCollection(mapsValue);
  const mapGrids = parseMapGridCollection(mapGridsValue);
  const map = createBlankMap(options);
  assertAvailableMapName(map.name, maps, mapGrids);
  maps.push(map);
  return maps;
}

/** Clone and append a uniquely named map to a detached collection. */
export function cloneMapInCollection(
  mapsValue: unknown,
  sourceName: string,
  options: CloneMapOptions,
  mapGridsValue: unknown = [],
): MapRecord[] {
  const maps = parseMapCollection(mapsValue);
  const mapGrids = parseMapGridCollection(mapGridsValue);
  const source = requireOneMap(maps, sourceName);
  const clone = cloneMapRecord(source.map, options);
  assertAvailableMapName(clone.name, maps, mapGrids);
  maps.push(clone);
  return maps;
}

/**
 * Resize every dense layer from the top-left origin. Sparse `i` positions are
 * translated to the new row stride; positions outside the retained rectangle
 * are removed. Omitted positions retain their omission when they remain index 0.
 */
export function resizeMapRecord(
  source: unknown,
  options: ResizeMapOptions,
): ResizeMapResult {
  const map = parseMapRecord(source);
  const width = positiveInteger(options.width, 'width');
  const height = positiveInteger(options.height, 'height');
  const denseLength = checkedDenseLength(width, height);
  const fill = options.fill ?? { tilesetIndex: 0, tileIndex: 0 };
  requireSafeInteger(fill.tilesetIndex, 'fill.tilesetIndex');
  requireSafeInteger(fill.tileIndex, 'fill.tileIndex');

  const overlapWidth = Math.min(map.width, width);
  const overlapHeight = Math.min(map.height, height);
  for (const [layer, previous] of Object.entries(map.tiles)) {
    const next = new Array<number>(denseLength);
    for (let offset = 0; offset < denseLength; offset += 2) {
      next[offset] = fill.tilesetIndex;
      next[offset + 1] = fill.tileIndex;
    }
    for (let y = 0; y < overlapHeight; y += 1) {
      for (let x = 0; x < overlapWidth; x += 1) {
        const previousOffset = (y * map.width + x) * 2;
        const nextOffset = (y * width + x) * 2;
        next[nextOffset] = previous[previousOffset]!;
        next[nextOffset + 1] = previous[previousOffset + 1]!;
      }
    }
    map.tiles[layer] = next;
  }

  const droppedPlacements = emptyDropCounts();
  for (const key of MAP_PLACEMENT_KEYS) {
    const placements = map[key];
    if (!placements) continue;
    const retained: MapTileReference[] = [];
    for (const placement of placements) {
      const oldIndex = placement.i ?? 0;
      const x = oldIndex % map.width;
      const y = Math.floor(oldIndex / map.width);
      if (
        oldIndex < 0 ||
        oldIndex >= map.width * map.height ||
        x >= width ||
        y >= height
      ) {
        droppedPlacements[key] += 1;
        continue;
      }
      const translated = y * width + x;
      if (Object.hasOwn(placement, 'i') || translated !== 0) {
        placement.i = translated;
      }
      retained.push(placement);
    }
    setPlacementList(map, key, retained);
  }

  map.width = width;
  map.height = height;
  return { map, droppedPlacements };
}

/** Add a zero-filled dense layer at the end of the preserved layer order. */
export function addMapLayer(source: unknown, layer: number): MapRecord {
  const map = parseMapRecord(source);
  requireSafeInteger(layer, 'layer');
  if (map.layers.includes(layer) || Object.hasOwn(map.tiles, String(layer))) {
    throw new MapLifecycleError(`layer ${layer} already exists`);
  }
  map.layers.push(layer);
  map.tiles[String(layer)] = new Array<number>(
    checkedDenseLength(map.width, map.height),
  ).fill(0);
  return map;
}

/**
 * Delete a dense layer and sparse placements on that effective layer. The final
 * layer cannot be removed because a map with no editable layer is not useful.
 */
export function deleteMapLayer(
  source: unknown,
  layer: number,
): DeleteMapLayerResult {
  const map = parseMapRecord(source);
  requireSafeInteger(layer, 'layer');
  if (!map.layers.includes(layer)) {
    throw new MapLifecycleError(`layer ${layer} does not exist`);
  }
  if (map.layers.length === 1) {
    throw new MapLifecycleError('cannot delete the final map layer');
  }

  map.layers = map.layers.filter((candidate) => candidate !== layer);
  delete map.tiles[String(layer)];
  const droppedPlacements = emptyDropCounts();
  for (const key of MAP_PLACEMENT_KEYS) {
    const placements = map[key];
    if (!placements) continue;
    const retained = placements.filter((placement) => {
      if ((placement.l ?? 0) !== layer) return true;
      droppedPlacements[key] += 1;
      return false;
    });
    setPlacementList(map, key, retained);
  }
  return { map, droppedPlacements };
}

/** Find every persisted reference affected by a map rename or deletion. */
export function previewMapReferences(
  database: MapDatabaseRecords,
  targetName: string,
): MapReferencePreview {
  const maps = parseMapCollection(database.maps);
  const mapGrids = parseMapGridCollection(database.mapGrids);
  return previewParsedReferences(maps, mapGrids, targetName);
}

/** Rename a map, every grid cell containing it, and unambiguous travel links. */
export function renameMapAcrossDatabase(
  database: MapDatabaseRecords,
  targetName: string,
  nextNameValue: string,
): MapDatabaseLifecycleResult {
  const maps = parseMapCollection(database.maps);
  const mapGrids = parseMapGridCollection(database.mapGrids);
  const preview = previewParsedReferences(maps, mapGrids, targetName);
  const nextName = requireApiName(nextNameValue, 'name');
  assertAvailableMapName(nextName, maps, mapGrids, preview.mapIndex);

  maps[preview.mapIndex]!.name = nextName;
  for (const reference of preview.gridCells) {
    mapGrids[reference.gridIndex]!.cells[reference.cellY]![reference.cellX] =
      nextName;
  }
  if (!preview.travelDestinationResolvesToGrid) {
    for (const reference of preview.travelTriggers) {
      maps[reference.mapIndex]!.travelTriggers![
        reference.triggerIndex
      ]!.destinationMapName = nextName;
    }
  }

  return { maps, mapGrids, preview };
}

/**
 * Delete a map, blank its grid cells, and clear unambiguous travel links. Other
 * trigger fields remain byte-for-byte equivalent when serialized as JSON data.
 */
export function deleteMapAcrossDatabase(
  database: MapDatabaseRecords,
  targetName: string,
): MapDatabaseLifecycleResult {
  const maps = parseMapCollection(database.maps);
  const mapGrids = parseMapGridCollection(database.mapGrids);
  const preview = previewParsedReferences(maps, mapGrids, targetName);

  for (const reference of preview.gridCells) {
    mapGrids[reference.gridIndex]!.cells[reference.cellY]![reference.cellX] =
      '';
  }
  if (!preview.travelDestinationResolvesToGrid) {
    for (const reference of preview.travelTriggers) {
      delete maps[reference.mapIndex]!.travelTriggers![reference.triggerIndex]!
        .destinationMapName;
    }
  }
  maps.splice(preview.mapIndex, 1);

  return { maps, mapGrids, preview };
}

function previewParsedReferences(
  maps: MapRecord[],
  mapGrids: MapGridRecord[],
  targetName: string,
): MapReferencePreview {
  const target = requireOneMap(maps, targetName);
  const gridCells: MapGridCellReference[] = [];
  const travelTriggers: MapTravelReference[] = [];
  mapGrids.forEach((grid, gridIndex) => {
    grid.cells.forEach((row, cellY) => {
      row.forEach((mapName, cellX) => {
        if (sameApiName(mapName, target.name)) {
          gridCells.push({ gridIndex, gridName: grid.name, cellX, cellY });
        }
      });
    });
  });
  maps.forEach((map, mapIndex) => {
    map.travelTriggers?.forEach((trigger, triggerIndex) => {
      if (sameApiName(trigger.destinationMapName, target.name)) {
        travelTriggers.push({ mapIndex, mapName: map.name, triggerIndex });
      }
    });
  });
  return {
    targetMapName: target.name,
    mapIndex: target.index,
    gridCells,
    travelTriggers,
    travelDestinationResolvesToGrid: mapGrids.some((grid) =>
      sameApiName(grid.name, target.name),
    ),
  };
}

function requireOneMap(
  maps: readonly MapRecord[],
  targetName: string,
): { map: MapRecord; index: number; name: string } {
  const target = requireApiName(targetName, 'targetName');
  const matches = maps
    .map((map, index) => ({ map, index }))
    .filter(({ map }) => sameApiName(map.name, target));
  if (matches.length !== 1) {
    throw new MapLifecycleError(
      matches.length
        ? `map name "${target}" is ambiguous`
        : `map "${target}" not found`,
    );
  }
  return { ...matches[0]!, name: matches[0]!.map.name.trim() };
}

function assertAvailableMapName(
  nameValue: string,
  maps: readonly MapRecord[],
  mapGrids: readonly MapGridRecord[],
  ignoredMapIndex = -1,
): void {
  const name = requireApiName(nameValue, 'name');
  if (
    maps.some(
      (map, index) => index !== ignoredMapIndex && sameApiName(map.name, name),
    )
  ) {
    throw new MapLifecycleError(`map "${name}" already exists`);
  }
  if (mapGrids.some((grid) => sameApiName(grid.name, name))) {
    throw new MapLifecycleError(
      `map name "${name}" conflicts with an existing map grid`,
    );
  }
}

function setPlacementList(
  map: MapRecord,
  key: MapPlacementKey,
  placements: MapTileReference[],
): void {
  (map as unknown as Record<MapPlacementKey, MapTileReference[]>)[key] =
    placements;
}

function emptyDropCounts(): PlacementDropCounts {
  return Object.fromEntries(
    MAP_PLACEMENT_KEYS.map((key) => [key, 0]),
  ) as PlacementDropCounts;
}

function checkedDenseLength(width: number, height: number): number {
  const length = width * height * 2;
  if (!Number.isSafeInteger(length) || length > MAX_ARRAY_LENGTH) {
    throw new MapLifecycleError(
      'map dimensions exceed JavaScript array limits',
    );
  }
  return length;
}

function positiveInteger(value: number, field: string): number {
  requireSafeInteger(value, field);
  if (value <= 0) {
    throw new MapLifecycleError(`${field} must be positive`);
  }
  return value;
}

function requireSafeInteger(value: number, field: string): void {
  if (!Number.isSafeInteger(value)) {
    throw new MapLifecycleError(`${field} must be a safe integer`);
  }
}

function requireApiName(value: string, field: string): string {
  const name = requireString(value, field).trim();
  if (!name) throw new MapLifecycleError(`${field} cannot be empty`);
  return name;
}

function requireString(value: unknown, field: string): string {
  if (typeof value !== 'string') {
    throw new MapLifecycleError(`${field} must be a string`);
  }
  return value;
}

function sameApiName(value: unknown, name: string): boolean {
  return typeof value === 'string' && value.trim() === name.trim();
}

function applyOptionalString(
  map: MapRecord,
  patch: MapMetadataPatch,
  key: 'label',
): void {
  if (!Object.hasOwn(patch, key)) return;
  const value = patch[key];
  if (value === null) {
    delete map[key];
  } else {
    map[key] = requireString(value, key);
  }
}

function applyOptionalPositiveInteger(
  map: MapRecord,
  patch: MapMetadataPatch,
  key: 'spriteWidth' | 'spriteHeight',
): void {
  if (!Object.hasOwn(patch, key)) return;
  const value = patch[key];
  if (value === null) {
    delete map[key];
  } else {
    map[key] = positiveInteger(value as number, key);
  }
}
