export const ASSET_REGISTRY = [
  {
    id: 'statusEffects',
    label: 'Status Effects',
    fileName: 'status-effects.json',
  },
  { id: 'abilities', label: 'Abilities', fileName: 'abilities.json' },
  { id: 'items', label: 'Items', fileName: 'items.json' },
  { id: 'spells', label: 'Spells', fileName: 'spells.json' },
  { id: 'characters', label: 'Characters', fileName: 'characters.json' },
  { id: 'maps', label: 'Maps', fileName: 'maps.json' },
  { id: 'mapGrids', label: 'Map Grids', fileName: 'map-grids.json' },
  { id: 'tilesets', label: 'Tilesets', fileName: 'tilesets.json' },
  {
    id: 'specialEvents',
    label: 'Special Events',
    fileName: 'special-events.json',
  },
] as const;

export type AssetDescriptor = (typeof ASSET_REGISTRY)[number];
export type AssetId = AssetDescriptor['id'];

/** Compatibility alias for server-side code that calls registry rows definitions. */
export const ASSET_DEFINITIONS = ASSET_REGISTRY;
export type AssetDefinition = AssetDescriptor;

export const ASSET_IDS: readonly AssetId[] = ASSET_REGISTRY.map(({ id }) => id);

const ASSET_ID_SET: ReadonlySet<string> = new Set(ASSET_IDS);

export function isAssetId(value: string): value is AssetId {
  return ASSET_ID_SET.has(value);
}

export function assetDescriptorForId(id: AssetId): AssetDescriptor {
  const descriptor = ASSET_REGISTRY.find((entry) => entry.id === id);
  if (!descriptor) {
    throw new Error(`Unknown asset id: ${id}`);
  }
  return descriptor;
}
