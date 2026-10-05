/**
 * Single source of truth for the JSON asset-database types the editor reads and
 * writes. Imported by both the Express server (under tsx) and the React client,
 * so this file must stay free of DOM and React types.
 */

export interface AssetTypeDescriptor {
  /** Stable id used in the API path (`/api/assets/:id`) and editor routes. */
  id: string;
  /** Human label shown on the Home page. */
  name: string;
  /** Filename under src/assets/db/. */
  file: string;
  /** Hash query param that deep-links to an entity, when the editor supports it. */
  routeParam?: string;
  /** Field that identifies one record when merging a newer file on save. */
  recordKey: 'id' | 'name';
}

export const ASSET_TYPES = [
  { id: 'itemTemplates', name: 'Item Templates', file: 'items.json', routeParam: 'item', recordKey: 'name' },
  { id: 'abilityTemplates', name: 'Ability Templates', file: 'abilities.json', routeParam: 'ability', recordKey: 'name' },
  { id: 'spellTemplates', name: 'Spell Templates', file: 'spells.json', routeParam: 'spell', recordKey: 'name' },
  { id: 'statusEffectTemplates', name: 'Status Effect Templates', file: 'status-effects.json', routeParam: 'statusEffect', recordKey: 'name' },
  { id: 'featTemplates', name: 'Feat Templates', file: 'feats.json', recordKey: 'id' },
  { id: 'questTemplates', name: 'Quests', file: 'quests.json', routeParam: 'quest', recordKey: 'id' },
  { id: 'characterTemplates', name: 'Character Templates', file: 'characters.json', routeParam: 'character', recordKey: 'name' },
  { id: 'specialEvents', name: 'Special Events', file: 'special-events.json', routeParam: 'event', recordKey: 'id' },
  { id: 'tilesetTemplates', name: 'Tileset Templates', file: 'tilesets.json', routeParam: 'tileset', recordKey: 'name' },
  { id: 'maps', name: 'Maps', file: 'maps.json', routeParam: 'map', recordKey: 'name' },
  { id: 'mapGrids', name: 'Map Grids', file: 'map-grids.json', routeParam: 'mapGrid', recordKey: 'name' },
] as const satisfies readonly AssetTypeDescriptor[];

export type AssetId = (typeof ASSET_TYPES)[number]['id'];

export const ASSET_FILE_BY_ID = Object.fromEntries(
  ASSET_TYPES.map((a) => [a.id, a.file]),
) as Record<AssetId, string>;

export const ASSET_RECORD_KEY_BY_ID = Object.fromEntries(
  ASSET_TYPES.map((a) => [a.id, a.recordKey]),
) as Record<AssetId, 'id' | 'name'>;

/** Deep-link hash query param per asset id, for those that support one. */
export const ASSET_ROUTE_PARAM_BY_ID = Object.fromEntries(
  ASSET_TYPES.flatMap((a) =>
    'routeParam' in a && a.routeParam ? [[a.id, a.routeParam]] : [],
  ),
) as Partial<Record<AssetId, string>>;

const ASSET_IDS = new Set<string>(ASSET_TYPES.map((a) => a.id));

export function isAssetId(id: string): id is AssetId {
  return ASSET_IDS.has(id);
}

/** Filename for an asset id, or undefined if the id is unknown. */
export function assetFileForId(id: string): string | undefined {
  return isAssetId(id) ? ASSET_FILE_BY_ID[id] : undefined;
}
