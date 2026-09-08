import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  MapCharacterPlacement,
  MapEventTriggerPlacement,
  MapItemPlacement,
  MapLightSourcePlacement,
  MapMarkerPlacement,
  MapTileItemEntry,
  MapTileOverridePlacement,
  MapTileRef,
  MapTravelTriggerPlacement,
  TileEventTrigger,
  TileLightSource,
  TileOverrides,
  TravelTrigger,
} from '../types/assets';
import {
  bumpMapDataRevision,
  getMapDataRevision,
} from '../tile-editor/editorState';

/** Legacy on-disk shape (pre-flat migration). */
export interface LegacyCarcerMapTemplate {
  name: string;
  label: string;
  type: CarcerMapTemplate['type'];
  width: number;
  height: number;
  spriteWidth: number;
  spriteHeight: number;
  levels: Record<string, CarcerMapTileTemplate[]>;
}

export function tileIndex(x: number, y: number, width: number): number {
  return y * width + x;
}

export function tileXY(
  i: number,
  width: number
): { x: number; y: number } {
  return { x: i % width, y: Math.floor(i / width) };
}

export function sameCell(a: MapTileRef, b: MapTileRef): boolean {
  return a.l === b.l && a.i === b.i;
}

export function layerKey(l: number): string {
  return String(l);
}

export function createEmptyTileGraphics(
  width: number,
  height: number
): number[] {
  const pairs = width * height * 2;
  const graphics = new Array<number>(pairs);
  for (let k = 0; k < width * height; k++) {
    graphics[k * 2] = 0;
    graphics[k * 2 + 1] = 0;
  }
  return graphics;
}

export function getOrAddTilesetIndex(
  map: CarcerMapTemplate,
  tilesetName: string
): number {
  const idx = map.tilesets.indexOf(tilesetName);
  if (idx >= 0) {
    return idx;
  }
  map.tilesets.push(tilesetName);
  return map.tilesets.length - 1;
}

export function getTileGraphic(
  map: CarcerMapTemplate,
  l: number,
  i: number
): { tilesetIndex: number; tileId: number } {
  const graphics = map.tiles[layerKey(l)];
  if (!graphics) {
    return { tilesetIndex: 0, tileId: 0 };
  }
  const offset = i * 2;
  return {
    tilesetIndex: graphics[offset] ?? 0,
    tileId: graphics[offset + 1] ?? 0,
  };
}

export function setTileGraphic(
  map: CarcerMapTemplate,
  l: number,
  i: number,
  tilesetIndex: number,
  tileId: number
): void {
  const key = layerKey(l);
  let graphics = map.tiles[key];
  const expectedLen = map.width * map.height * 2;
  if (!graphics || graphics.length !== expectedLen) {
    graphics = createEmptyTileGraphics(map.width, map.height);
    map.tiles[key] = graphics;
  }
  const offset = i * 2;
  graphics[offset] = tilesetIndex;
  graphics[offset + 1] = tileId;
}

export function ensureMapLayers(map: CarcerMapTemplate): void {
  if (!map.tilesets?.length) {
    map.tilesets = [''];
  } else if (map.tilesets[0] !== '') {
    map.tilesets = ['', ...map.tilesets.filter((t) => t !== '')];
  }
  if (!map.layers?.length) {
    map.layers = [0];
  }
  if (!map.tiles) {
    map.tiles = {};
  }
  for (const kind of PLACEMENT_KINDS) {
    if (!getPlacementList(map, kind.listKey)) {
      setPlacementList(map, kind.listKey, []);
    }
  }
  for (const l of map.layers) {
    const key = layerKey(l);
    const expectedLen = map.width * map.height * 2;
    if (
      !map.tiles[key] ||
      map.tiles[key].length !== expectedLen
    ) {
      map.tiles[key] = createEmptyTileGraphics(map.width, map.height);
    }
  }
}

export function createTilesForLayer(map: CarcerMapTemplate, l: number): void {
  ensureMapLayers(map);
  if (!map.layers.includes(l)) {
    map.layers = [...map.layers, l].sort((a, b) => b - a);
  }
  map.tiles[layerKey(l)] = createEmptyTileGraphics(map.width, map.height);
}

export function isLegacyMap(
  raw: CarcerMapTemplate | LegacyCarcerMapTemplate
): raw is LegacyCarcerMapTemplate {
  return (
    'levels' in raw &&
    raw.levels !== undefined &&
    !(
      'tilesets' in raw &&
      Array.isArray((raw as unknown as CarcerMapTemplate).tilesets)
    )
  );
}

export function migrateLegacyMap(
  legacy: LegacyCarcerMapTemplate
): CarcerMapTemplate {
  const map: CarcerMapTemplate = {
    name: legacy.name,
    label: legacy.label,
    type: legacy.type,
    width: legacy.width,
    height: legacy.height,
    spriteWidth: legacy.spriteWidth,
    spriteHeight: legacy.spriteHeight,
    tilesets: [''],
    layers: [],
    tiles: {},
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };

  const levelKeys = Object.keys(legacy.levels).sort(
    (a, b) => parseInt(b, 10) - parseInt(a, 10)
  );
  map.layers = levelKeys.map((k) => parseInt(k, 10));

  for (const levelKey of levelKeys) {
    const l = parseInt(levelKey, 10);
    const legacyTiles = legacy.levels[levelKey] ?? [];
    const graphics = createEmptyTileGraphics(map.width, map.height);

    legacyTiles.forEach((tile, i) => {
      const tsIdx = getOrAddTilesetIndex(map, tile.tilesetName ?? '');
      graphics[i * 2] = tsIdx;
      graphics[i * 2 + 1] = tile.tileId ?? 0;

      appendTilePlacements(map, l, i, tile);
    });

    map.tiles[layerKey(l)] = graphics;
  }

  ensureMapLayers(map);
  return map;
}

export function normalizeMapOnLoad(
  raw: CarcerMapTemplate | LegacyCarcerMapTemplate
): CarcerMapTemplate {
  const map = isLegacyMap(raw) ? migrateLegacyMap(raw) : { ...raw };
  ensureMapLayers(map);
  return map;
}

/**
 * One placement "kind" — a flat list on {@link CarcerMapTemplate} that mirrors a
 * field on {@link CarcerMapTileTemplate}. Adding a new per-tile property means
 * adding one entry here instead of editing six functions by hand.
 */
export interface PlacementKind {
  /** Key on CarcerMapTemplate holding the flat placement list. */
  listKey:
    | 'characters'
    | 'items'
    | 'markers'
    | 'eventTriggers'
    | 'travelTriggers'
    | 'tileOverrides'
    | 'lightSources';
  /** Key on CarcerMapTileTemplate holding the materialized value. */
  tileKey: keyof CarcerMapTileTemplate;
  /** 'many' -> array on the tile; 'one' -> single optional value. */
  cardinality: 'many' | 'one';
  /** Placement entry -> value stored on the materialized tile. */
  toTile(entry: any): any;
  /** Tile value -> placement payloads, without l/i (added by the caller). */
  fromTile(value: any): any[];
}

/**
 * Preserve the HIDDEN asymmetry: materialization fills in a 'HIDDEN' default when
 * reading, but the write side omits the field entirely when it equals 'HIDDEN',
 * keeping it out of maps.json.
 */
function keepOverlayVisibility(
  visibility: string | undefined
): { overlayVisibility: string } | Record<string, never> {
  return visibility && visibility !== 'HIDDEN'
    ? { overlayVisibility: visibility }
    : {};
}

export const PLACEMENT_KINDS: readonly PlacementKind[] = [
  {
    listKey: 'characters',
    tileKey: 'characters',
    cardinality: 'many',
    toTile: (c) => c.name,
    fromTile: (names: string[]) =>
      (names ?? []).filter(Boolean).map((name) => ({ name })),
  },
  {
    listKey: 'items',
    tileKey: 'items',
    cardinality: 'many',
    toTile: (it) => ({ name: it.name, quantity: it.quantity }),
    fromTile: (entries: MapTileItemEntry[]) =>
      (entries ?? []).map((e) => ({ name: e.name, quantity: e.quantity ?? 1 })),
  },
  {
    listKey: 'markers',
    tileKey: 'markers',
    cardinality: 'many',
    toTile: (m) => m.name,
    fromTile: (names: string[]) =>
      (names ?? []).filter(Boolean).map((name) => ({ name })),
  },
  {
    listKey: 'eventTriggers',
    tileKey: 'eventTrigger',
    cardinality: 'one',
    toTile: (e) => ({
      eventId: e.eventId,
      requiresNonCombat: e.requiresNonCombat,
      requiresLook: e.requiresLook,
      overlayVisibility: e.overlayVisibility ?? 'HIDDEN',
    }),
    fromTile: (v: TileEventTrigger) => [
      {
        eventId: v.eventId,
        requiresNonCombat: v.requiresNonCombat,
        requiresLook: v.requiresLook,
        ...keepOverlayVisibility(v.overlayVisibility),
      },
    ],
  },
  {
    listKey: 'travelTriggers',
    tileKey: 'travelTrigger',
    cardinality: 'one',
    toTile: (t) => ({
      destinationMapName: t.destinationMapName,
      destinationMarkerName: t.destinationMarkerName,
      destinationX: t.destinationX,
      destinationY: t.destinationY,
      destinationLayer: t.destinationLayer ?? 0,
      requiresAction: t.requiresAction ?? false,
      overlayVisibility: t.overlayVisibility ?? 'HIDDEN',
    }),
    fromTile: (v: TravelTrigger) => [
      {
        destinationMapName: v.destinationMapName,
        destinationMarkerName: v.destinationMarkerName,
        destinationX: v.destinationX,
        destinationY: v.destinationY,
        destinationLayer: v.destinationLayer ?? 0,
        requiresAction: v.requiresAction ?? false,
        ...keepOverlayVisibility(v.overlayVisibility),
      },
    ],
  },
  {
    listKey: 'tileOverrides',
    tileKey: 'tileOverrides',
    cardinality: 'one',
    toTile: (o) => ({ ...o.overrides }),
    fromTile: (v: TileOverrides) => [{ overrides: v }],
  },
  {
    listKey: 'lightSources',
    tileKey: 'lightSource',
    cardinality: 'one',
    toTile: (ls) => ({
      angle: ls.angle,
      intensity: ls.intensity,
      radius: ls.radius,
    }),
    fromTile: (v: TileLightSource) => [{ ...v }],
  },
];

type PlacementListKey = PlacementKind['listKey'];

function getPlacementList(
  map: CarcerMapTemplate,
  key: PlacementListKey
): MapTileRef[] {
  return map[key] as unknown as MapTileRef[];
}

function setPlacementList(
  map: CarcerMapTemplate,
  key: PlacementListKey,
  list: MapTileRef[]
): void {
  (map as unknown as Record<PlacementListKey, MapTileRef[]>)[key] = list;
}

/** One pass over a placement list, bucketed by tile index, for a single layer. */
function indexByTileIndex<T extends MapTileRef>(
  list: T[] | undefined,
  l: number
): Map<number, T[]> {
  const byIndex = new Map<number, T[]>();
  for (const entry of list ?? []) {
    if (entry.l !== l) continue;
    const bucket = byIndex.get(entry.i);
    if (bucket) bucket.push(entry);
    else byIndex.set(entry.i, [entry]);
  }
  return byIndex;
}

/**
 * Append every placement kind for a single materialized tile to the map's flat
 * lists. Shared by writeLayerFromTiles and migrateLegacyMap so the two paths
 * cannot drift.
 */
function appendTilePlacements(
  map: CarcerMapTemplate,
  l: number,
  i: number,
  tile: CarcerMapTileTemplate
): void {
  const source = tile as unknown as Record<string, unknown>;
  for (const kind of PLACEMENT_KINDS) {
    const value = source[kind.tileKey];
    if (kind.cardinality === 'one' && value == null) continue;
    const list = getPlacementList(map, kind.listKey);
    for (const payload of kind.fromTile(value)) {
      list.push({ l, i, ...payload });
    }
  }
}

export function materializeLayer(
  map: CarcerMapTemplate,
  l: number
): CarcerMapTileTemplate[] {
  ensureMapLayers(map);
  const count = map.width * map.height;
  const graphics =
    map.tiles[layerKey(l)] ??
    createEmptyTileGraphics(map.width, map.height);
  const result: CarcerMapTileTemplate[] = [];

  // One pass per kind, bucketed by tile index — O(tiles + placements).
  const indexes = PLACEMENT_KINDS.map((kind) =>
    indexByTileIndex(getPlacementList(map, kind.listKey), l)
  );

  for (let i = 0; i < count; i++) {
    const tsIdx = graphics[i * 2] ?? 0;
    const tileId = graphics[i * 2 + 1] ?? 0;
    const tile = {
      tilesetName: map.tilesets[tsIdx] ?? '',
      tileId,
    } as CarcerMapTileTemplate;
    const writable = tile as unknown as Record<string, unknown>;

    for (let k = 0; k < PLACEMENT_KINDS.length; k++) {
      const kind = PLACEMENT_KINDS[k];
      const bucket = indexes[k].get(i);
      if (kind.cardinality === 'many') {
        writable[kind.tileKey] = (bucket ?? []).map((e) => kind.toTile(e));
      } else if (bucket && bucket.length > 0) {
        writable[kind.tileKey] = kind.toTile(bucket[0]);
      }
    }

    result.push(tile);
  }

  return result;
}

export function writeLayerFromTiles(
  map: CarcerMapTemplate,
  l: number,
  tiles: CarcerMapTileTemplate[]
): void {
  ensureMapLayers(map);
  const graphics = createEmptyTileGraphics(map.width, map.height);

  for (const kind of PLACEMENT_KINDS) {
    setPlacementList(
      map,
      kind.listKey,
      getPlacementList(map, kind.listKey).filter((entry) => entry.l !== l)
    );
  }

  const count = Math.min(tiles.length, map.width * map.height);
  for (let i = 0; i < count; i++) {
    const tile = tiles[i];
    const tsIdx = getOrAddTilesetIndex(map, tile.tilesetName ?? '');
    graphics[i * 2] = tsIdx;
    graphics[i * 2 + 1] = tile.tileId ?? 0;

    appendTilePlacements(map, l, i, tile);
  }

  map.tiles[layerKey(l)] = graphics;
  if (!map.layers.includes(l)) {
    map.layers = [...map.layers, l].sort((a, b) => b - a);
  }
  bumpMapDataRevision(map.name);
}

/**
 * Cache of materialized layers, keyed on `name|revision|layer` rather than map
 * object identity, so an ordinary React `{ ...map }` spread no longer discards
 * it. Not a WeakMap, so it needs an explicit eviction rule (invariant I6).
 */
const MAX_CACHED_LAYERS = 32;
const layerViewCache = new Map<string, CarcerMapTileTemplate[]>();

function layerCacheKey(name: string, l: number): string {
  return name + '|' + getMapDataRevision(name) + '|' + l;
}

export function getMaterializedLayer(
  map: CarcerMapTemplate,
  l: number
): CarcerMapTileTemplate[] {
  const key = layerCacheKey(map.name, l);
  const hit = layerViewCache.get(key);
  if (hit) {
    return hit;
  }

  const tiles = materializeLayer(map, l);
  layerViewCache.set(key, tiles);
  if (layerViewCache.size > MAX_CACHED_LAYERS) {
    // Map preserves insertion order; drop the oldest entry.
    const oldest = layerViewCache.keys().next().value;
    if (oldest !== undefined) {
      layerViewCache.delete(oldest);
    }
  }
  return tiles;
}

export function commitMaterializedLayer(
  map: CarcerMapTemplate,
  l: number
): void {
  // Read before writeLayerFromTiles bumps the revision out from under the key.
  const tiles = layerViewCache.get(layerCacheKey(map.name, l));
  if (!tiles) {
    return;
  }
  writeLayerFromTiles(map, l, tiles);
}

export function sortedLayerKeys(map: CarcerMapTemplate): number[] {
  return [...map.layers].sort((a, b) => b - a);
}

/** Layer above/below in the sidebar list (sorted high → low). */
export function getAdjacentLayer(
  map: CarcerMapTemplate,
  currentLevel: number,
  direction: 'up' | 'down',
): number | null {
  const layers = sortedLayerKeys(map);
  const index = layers.indexOf(currentLevel);
  if (index === -1) {
    return null;
  }
  if (direction === 'up') {
    return index > 0 ? layers[index - 1]! : null;
  }
  return index < layers.length - 1 ? layers[index + 1]! : null;
}

export function createDefaultMapTemplate(): CarcerMapTemplate {
  const map: CarcerMapTemplate = {
    name: '',
    label: '',
    type: 'TOWN',
    width: 40,
    height: 40,
    spriteWidth: 28,
    spriteHeight: 32,
    tilesets: [''],
    layers: [0],
    tiles: {},
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };
  createTilesForLayer(map, 0);
  return map;
}

/** Ensure layer-0 tile graphics exist for a newly created map. */
export function prepareNewMapForEditor(map: CarcerMapTemplate): CarcerMapTemplate {
  const next: CarcerMapTemplate = { ...map };
  const expectedTileCount = next.width * next.height;
  const level0 = next.tiles['0'];
  if (!level0?.length || level0.length !== expectedTileCount * 2) {
    createTilesForLayer(next, 0);
  }
  return next;
}

export function resizeMaterializedLevelTiles(
  tiles: CarcerMapTileTemplate[],
  prevWidth: number,
  prevHeight: number,
  newWidth: number,
  newHeight: number,
  createEmpty: () => CarcerMapTileTemplate
): CarcerMapTileTemplate[] {
  const existingTiles = new Map<string, CarcerMapTileTemplate>();
  for (let y = 0; y < prevHeight; y++) {
    for (let x = 0; x < prevWidth; x++) {
      const tile = tiles[y * prevWidth + x];
      if (tile) {
        existingTiles.set(`${x},${y}`, tile);
      }
    }
  }

  const nextTiles: CarcerMapTileTemplate[] = [];
  for (let y = 0; y < newHeight; y++) {
    for (let x = 0; x < newWidth; x++) {
      nextTiles.push(existingTiles.get(`${x},${y}`) ?? createEmpty());
    }
  }
  return nextTiles;
}

export function resizeMap(
  map: CarcerMapTemplate,
  prevWidth: number,
  prevHeight: number,
  newWidth: number,
  newHeight: number,
  createEmpty: () => CarcerMapTileTemplate
): CarcerMapTemplate {
  const next: CarcerMapTemplate = {
    ...map,
    width: newWidth,
    height: newHeight,
    tiles: { ...map.tiles },
  };
  bumpMapDataRevision(map.name);

  for (const l of map.layers) {
    const tiles = materializeLayer(map, l);
    const resized = resizeMaterializedLevelTiles(
      tiles,
      prevWidth,
      prevHeight,
      newWidth,
      newHeight,
      createEmpty
    );
    writeLayerFromTiles(next, l, resized);
  }

  const maxI = newWidth * newHeight;
  const inBounds = (p: MapTileRef) => p.i >= 0 && p.i < maxI;
  for (const kind of PLACEMENT_KINDS) {
    setPlacementList(
      next,
      kind.listKey,
      getPlacementList(next, kind.listKey).filter(inBounds)
    );
  }
  ensureMapLayers(next);
  return next;
}

export function deleteMapLayer(map: CarcerMapTemplate, l: number): CarcerMapTemplate {
  const next: CarcerMapTemplate = {
    ...map,
    layers: map.layers.filter((layer) => layer !== l),
    tiles: { ...map.tiles },
  };
  delete next.tiles[layerKey(l)];
  for (const kind of PLACEMENT_KINDS) {
    setPlacementList(
      next,
      kind.listKey,
      getPlacementList(next, kind.listKey).filter((entry) => entry.l !== l)
    );
  }
  if (!next.layers.length) {
    next.layers = [0];
    createTilesForLayer(next, 0);
  }
  bumpMapDataRevision(map.name);
  return next;
}

export function addMapLayer(
  map: CarcerMapTemplate,
  l: number
): CarcerMapTemplate {
  const next = { ...map, tiles: { ...map.tiles } };
  createTilesForLayer(next, l);
  bumpMapDataRevision(map.name);
  return next;
}

export type {
  MapCharacterPlacement,
  MapEventTriggerPlacement,
  MapItemPlacement,
  MapLightSourcePlacement,
  MapMarkerPlacement,
  MapTileOverridePlacement,
  MapTileRef,
  MapTravelTriggerPlacement,
  TileEventTrigger,
  TileLightSource,
  TileOverrides,
  TravelTrigger,
};
