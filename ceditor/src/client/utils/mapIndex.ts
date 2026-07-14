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
  if (!map.characters) {
    map.characters = [];
  }
  if (!map.items) {
    map.items = [];
  }
  if (!map.markers) {
    map.markers = [];
  }
  if (!map.eventTriggers) {
    map.eventTriggers = [];
  }
  if (!map.travelTriggers) {
    map.travelTriggers = [];
  }
  if (!map.tileOverrides) {
    map.tileOverrides = [];
  }
  if (!map.lightSources) {
    map.lightSources = [];
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
    !('tilesets' in raw && Array.isArray((raw as CarcerMapTemplate).tilesets))
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

      for (const name of tile.characters ?? []) {
        if (name) {
          map.characters.push({ l, i, name });
        }
      }
      for (const item of tile.items ?? []) {
        map.items.push({
          l,
          i,
          name: item.name,
          quantity: item.quantity ?? 1,
        });
      }
      for (const name of tile.markers ?? []) {
        if (name) {
          map.markers.push({ l, i, name });
        }
      }
      if (tile.eventTrigger) {
        map.eventTriggers.push({
          l,
          i,
          ...tile.eventTrigger,
        });
      }
      if (tile.travelTrigger) {
        map.travelTriggers.push({
          l,
          i,
          ...tile.travelTrigger,
        });
      }
      if (tile.tileOverrides) {
        map.tileOverrides.push({ l, i, overrides: tile.tileOverrides });
      }
      if (tile.lightSource) {
        map.lightSources.push({ l, i, ...tile.lightSource });
      }
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

function placementsAt<L extends MapTileRef>(
  list: L[],
  l: number,
  i: number
): L[] {
  return list.filter((entry) => entry.l === l && entry.i === i);
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

  for (let i = 0; i < count; i++) {
    const tsIdx = graphics[i * 2] ?? 0;
    const tileId = graphics[i * 2 + 1] ?? 0;
    const tile: CarcerMapTileTemplate = {
      tilesetName: map.tilesets[tsIdx] ?? '',
      tileId,
      characters: placementsAt(map.characters, l, i).map((c) => c.name),
      items: placementsAt(map.items, l, i).map(
        (item): MapTileItemEntry => ({
          name: item.name,
          quantity: item.quantity,
        })
      ),
      markers: placementsAt(map.markers, l, i).map((m) => m.name),
    };

    const event = placementsAt(map.eventTriggers, l, i)[0];
    if (event) {
      tile.eventTrigger = {
        eventId: event.eventId,
        isNonCombatTrigger: event.isNonCombatTrigger,
        isLookTrigger: event.isLookTrigger,
      };
    }

    const travel = placementsAt(map.travelTriggers, l, i)[0];
    if (travel) {
      tile.travelTrigger = {
        destinationMapName: travel.destinationMapName,
        destinationMarkerName: travel.destinationMarkerName,
        destinationX: travel.destinationX,
        destinationY: travel.destinationY,
        destinationLayer: travel.destinationLayer ?? 0,
      };
    }

    const override = placementsAt(map.tileOverrides, l, i)[0];
    if (override) {
      tile.tileOverrides = { ...override.overrides };
    }

    const light = placementsAt(map.lightSources, l, i)[0];
    if (light) {
      tile.lightSource = {
        angle: light.angle,
        intensity: light.intensity,
        radius: light.radius,
      };
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

  map.characters = map.characters.filter((c) => c.l !== l);
  map.items = map.items.filter((item) => item.l !== l);
  map.markers = map.markers.filter((m) => m.l !== l);
  map.eventTriggers = map.eventTriggers.filter((e) => e.l !== l);
  map.travelTriggers = map.travelTriggers.filter((t) => t.l !== l);
  map.tileOverrides = map.tileOverrides.filter((o) => o.l !== l);
  map.lightSources = map.lightSources.filter((ls) => ls.l !== l);

  const count = Math.min(tiles.length, map.width * map.height);
  for (let i = 0; i < count; i++) {
    const tile = tiles[i];
    const tsIdx = getOrAddTilesetIndex(map, tile.tilesetName ?? '');
    graphics[i * 2] = tsIdx;
    graphics[i * 2 + 1] = tile.tileId ?? 0;

    for (const name of tile.characters ?? []) {
      if (name) {
        map.characters.push({ l, i, name });
      }
    }
    for (const item of tile.items ?? []) {
      map.items.push({
        l,
        i,
        name: item.name,
        quantity: item.quantity ?? 1,
      });
    }
    for (const name of tile.markers ?? []) {
      if (name) {
        map.markers.push({ l, i, name });
      }
    }
    if (tile.eventTrigger) {
      map.eventTriggers.push({
        l,
        i,
        ...tile.eventTrigger,
      });
    }
    if (tile.travelTrigger) {
      map.travelTriggers.push({
        l,
        i,
        ...tile.travelTrigger,
      });
    }
    if (tile.tileOverrides) {
      map.tileOverrides.push({ l, i, overrides: tile.tileOverrides });
    }
    if (tile.lightSource) {
      map.lightSources.push({ l, i, ...tile.lightSource });
    }
  }

  map.tiles[layerKey(l)] = graphics;
  if (!map.layers.includes(l)) {
    map.layers = [...map.layers, l].sort((a, b) => b - a);
  }
}

const layerViewCache = new WeakMap<
  CarcerMapTemplate,
  Map<number, CarcerMapTileTemplate[]>
>();

export function getMaterializedLayer(
  map: CarcerMapTemplate,
  l: number
): CarcerMapTileTemplate[] {
  let byLayer = layerViewCache.get(map);
  if (!byLayer) {
    byLayer = new Map();
    layerViewCache.set(map, byLayer);
  }
  if (!byLayer.has(l)) {
    byLayer.set(l, materializeLayer(map, l));
  }
  return byLayer.get(l)!;
}

export function invalidateMaterializedLayer(
  map: CarcerMapTemplate,
  l?: number
): void {
  const byLayer = layerViewCache.get(map);
  if (!byLayer) {
    return;
  }
  if (l === undefined) {
    byLayer.clear();
  } else {
    byLayer.delete(l);
  }
}

export function commitMaterializedLayer(
  map: CarcerMapTemplate,
  l: number
): void {
  const byLayer = layerViewCache.get(map);
  const tiles = byLayer?.get(l);
  if (!tiles) {
    return;
  }
  writeLayerFromTiles(map, l, tiles);
  byLayer.set(l, materializeLayer(map, l));
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
  invalidateMaterializedLayer(map);

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
  next.characters = next.characters.filter(inBounds);
  next.items = next.items.filter(inBounds);
  next.markers = next.markers.filter(inBounds);
  next.eventTriggers = next.eventTriggers.filter(inBounds);
  next.travelTriggers = next.travelTriggers.filter(inBounds);
  next.tileOverrides = next.tileOverrides.filter(inBounds);
  next.lightSources = next.lightSources.filter(inBounds);
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
  next.characters = next.characters.filter((c) => c.l !== l);
  next.items = next.items.filter((item) => item.l !== l);
  next.markers = next.markers.filter((m) => m.l !== l);
  next.eventTriggers = next.eventTriggers.filter((e) => e.l !== l);
  next.travelTriggers = next.travelTriggers.filter((t) => t.l !== l);
  next.tileOverrides = next.tileOverrides.filter((o) => o.l !== l);
  next.lightSources = next.lightSources.filter((ls) => ls.l !== l);
  if (!next.layers.length) {
    next.layers = [0];
    createTilesForLayer(next, 0);
  }
  invalidateMaterializedLayer(map);
  return next;
}

export function addMapLayer(
  map: CarcerMapTemplate,
  l: number
): CarcerMapTemplate {
  const next = { ...map, tiles: { ...map.tiles } };
  createTilesForLayer(next, l);
  invalidateMaterializedLayer(map);
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
