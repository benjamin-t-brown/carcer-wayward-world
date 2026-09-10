import { parseMapRecord } from './mapParser.js';
import {
  MAP_PLACEMENT_KEYS,
  type MapPlacementKey,
  type MapRecord,
  type MapTileReference,
} from './types.js';

export type MapTilePlacementBundle = Partial<
  Record<MapPlacementKey, MapTileReference[]>
>;

export function tilePlacementsAt(
  source: unknown,
  layer: number,
  index: number,
): MapTilePlacementBundle {
  const map = parseMapRecord(source);
  assertCell(map, layer, index);
  const result: MapTilePlacementBundle = {};
  for (const key of MAP_PLACEMENT_KEYS) {
    const matching = map[key]?.filter(
      (placement) =>
        (placement.l ?? 0) === layer && (placement.i ?? 0) === index,
    );
    if (matching?.length) result[key] = matching;
  }
  return result;
}

/** Replace only sparse metadata at one tile; all other records retain order. */
export function replaceTilePlacements(
  source: unknown,
  layer: number,
  index: number,
  bundle: MapTilePlacementBundle,
): MapRecord {
  const map = parseMapRecord(source);
  assertCell(map, layer, index);
  for (const key of MAP_PLACEMENT_KEYS) {
    if (!Object.hasOwn(bundle, key)) continue;
    const replacements = bundle[key];
    if (!Array.isArray(replacements)) {
      throw new TypeError(`${key} must be an array when provided`);
    }
    const retained = (map[key] ?? []).filter(
      (placement) =>
        (placement.l ?? 0) !== layer || (placement.i ?? 0) !== index,
    );
    const positioned = replacements.map((placement) => ({
      ...structuredClone(placement),
      l: layer,
      i: index,
    }));
    (map as unknown as Record<MapPlacementKey, MapTileReference[]>)[key] = [
      ...retained,
      ...positioned,
    ];
  }
  return parseMapRecord(map);
}

function assertCell(map: MapRecord, layer: number, index: number): void {
  if (!map.layers.includes(layer))
    throw new RangeError(`layer ${layer} is missing`);
  if (
    !Number.isSafeInteger(index) ||
    index < 0 ||
    index >= map.width * map.height
  ) {
    throw new RangeError(`tile index ${index} is out of bounds`);
  }
}
