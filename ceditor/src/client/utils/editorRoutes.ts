import { AssetId, isAssetId } from '../../shared/assetRegistry';

/**
 * User-facing page for every managed JSON asset type. Keeping this exhaustive
 * over AssetId makes a newly managed asset a compile-time routing decision.
 */
export const EDITOR_ROUTE_BY_ASSET_ID: Record<AssetId, string> = {
  itemTemplates: '/editor/itemTemplates',
  abilityTemplates: '/editor/abilityTemplates',
  spellTemplates: '/editor/spellTemplates',
  statusEffectTemplates: '/editor/statusEffectTemplates',
  characterTemplates: '/editor/characterTemplates',
  specialEvents: '/editor/specialEvents',
  tilesetTemplates: '/editor/tilesetTemplates',
  maps: '/editor/maps',
  mapGrids: '/editor/mapGrids',
};

const ASSET_ID_BY_EDITOR_ROUTE = new Map<string, AssetId>(
  Object.entries(EDITOR_ROUTE_BY_ASSET_ID).map(([id, route]) => [
    route,
    id as AssetId,
  ]),
);

export function editorRouteForAssetId(id: string): string | undefined {
  return isAssetId(id) ? EDITOR_ROUTE_BY_ASSET_ID[id] : undefined;
}

export function assetIdForEditorRoute(path: string): AssetId | undefined {
  return ASSET_ID_BY_EDITOR_ROUTE.get(path);
}
