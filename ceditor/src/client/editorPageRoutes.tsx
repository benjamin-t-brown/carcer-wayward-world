import { lazy, type ComponentType, type LazyExoticComponent } from 'react';
import type { AssetId } from '../shared/assetRegistry';
import { assetIdForEditorRoute } from './utils/editorRoutes';

export interface EditorPageProps {
  routeParams?: URLSearchParams;
}

export type LazyEditorPage = LazyExoticComponent<
  ComponentType<EditorPageProps>
>;

function lazyEditorPage(
  load: () => Promise<ComponentType<EditorPageProps>>,
): LazyEditorPage {
  return lazy(async () => ({ default: await load() }));
}

export const EDITOR_PAGE_BY_ASSET_ID = {
  itemTemplates: lazyEditorPage(
    async () => (await import('./pages/ItemTemplates')).ItemTemplates,
  ),
  abilityTemplates: lazyEditorPage(
    async () => (await import('./pages/AbilityTemplates')).AbilityTemplates,
  ),
  spellTemplates: lazyEditorPage(
    async () => (await import('./pages/SpellTemplates')).SpellTemplates,
  ),
  statusEffectTemplates: lazyEditorPage(
    async () =>
      (await import('./pages/StatusEffectTemplates')).StatusEffectTemplates,
  ),
  characterTemplates: lazyEditorPage(
    async () => (await import('./pages/CharacterTemplates')).CharacterTemplates,
  ),
  specialEvents: lazyEditorPage(
    async () => (await import('./pages/SpecialEvents')).SpecialEvents,
  ),
  tilesetTemplates: lazyEditorPage(
    async () => (await import('./pages/TilesetTemplates')).TilesetTemplates,
  ),
  maps: lazyEditorPage(async () => (await import('./pages/Maps')).Maps),
  mapGrids: lazyEditorPage(
    async () => (await import('./pages/MapGrids')).MapGrids,
  ),
} satisfies Record<AssetId, LazyEditorPage>;

export const SOUND_EFFECTS_ROUTE = '/editor/soundEffects';
const SoundEffectsPage = lazyEditorPage(
  async () => (await import('./pages/SoundEffects')).SoundEffects,
);

export function editorPageForRoute(path: string): LazyEditorPage | undefined {
  const assetId = assetIdForEditorRoute(path);
  if (assetId) {
    return EDITOR_PAGE_BY_ASSET_ID[assetId];
  }
  return path === SOUND_EFFECTS_ROUTE ? SoundEffectsPage : undefined;
}
