import React from 'react';
import { createRoot, type Root } from 'react-dom/client';

import App from '../App';
import { AssetsProvider } from '../contexts/AssetsContext';
import { SDL2WAssetsProvider } from '../contexts/SDL2WAssetsContext';
import type { EditorBootstrapData } from './loadEditorData';

export function renderEditor(
  container: HTMLElement,
  data: EditorBootstrapData,
): Root {
  container.replaceChildren();
  const root = createRoot(container);
  root.render(
    <React.StrictMode>
      <SDL2WAssetsProvider
        sprites={data.sprites}
        spriteMap={data.spriteMap}
        animations={data.animations}
        animationMap={data.animationMap}
        sounds={data.sounds}
        soundMap={data.soundMap}
        pictures={data.pictures}
      >
        <AssetsProvider
          session={data.databaseSession}
          initialItems={data.items}
          initialCharacters={data.characters}
          initialAbilities={data.abilities}
          initialSpells={data.spells}
          initialStatusEffects={data.statusEffects}
          initialFeats={data.feats}
          initialTilesets={data.tilesets}
          initialGameEvents={data.gameEvents}
          initialMaps={data.maps}
          initialMapGrids={data.mapGrids}
        >
          <App assetTypes={data.assetTypes} />
        </AssetsProvider>
      </SDL2WAssetsProvider>
    </React.StrictMode>,
  );
  return root;
}

export function renderLoading(container: HTMLElement): void {
  const loading = document.createElement('div');
  loading.className = 'loading';
  loading.textContent = 'Loading assets...';
  container.replaceChildren(loading);
}

export function renderBootstrapError(
  container: HTMLElement,
  error: unknown,
): void {
  const wrapper = document.createElement('div');
  wrapper.className = 'container';
  const message = document.createElement('div');
  message.className = 'error';
  message.textContent = `Failed to load application: ${
    error instanceof Error ? error.message : 'Unknown error'
  }`;
  wrapper.append(message);
  container.replaceChildren(wrapper);
}
