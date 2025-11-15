import React from 'react';
import { createRoot } from 'react-dom/client';
import App from './App';
import {
  Animation,
  loadSpritesAndAnimations,
  Sprite,
} from './utils/assetLoader';
import { SDL2WAssetsProvider } from './contexts/SDL2WAssetsContext';

interface AssetType {
  id: string;
  name: string;
  file: string;
}

async function loadAssetTypes(): Promise<AssetType[]> {
  const response = await fetch('/api/assets/types');
  if (!response.ok) {
    throw new Error('Failed to load asset types');
  }
  return response.json();
}

async function loadSDL2WAssetFiles(): Promise<any> {
  const response = await fetch('/api/sdl2w-assets');
  if (!response.ok) {
    throw new Error('Failed to load SDL2W assets');
  }
  return response.json();
}

async function load(): Promise<{
  assetTypes: AssetType[];
  sprites: Sprite[];
  animations: Animation[];
  pictures: Record<string, string>;
}> {
  const assetTypes = await loadAssetTypes();
  const sdl2wAssetFIles = await loadSDL2WAssetFiles();
  const { sprites, animations, pictures } = await loadSpritesAndAnimations(
    sdl2wAssetFIles
  );
  console.log('loaded', { assetTypes, sprites, animations, pictures });
  return { assetTypes, sprites, animations, pictures };
}

async function init() {
  try {
    const { assetTypes, sprites, animations, pictures } = await load();
    const container = document.getElementById('root');

    if (!container) {
      throw new Error('Root element not found');
    }

    const root = createRoot(container);
    root.render(
      <React.StrictMode>
        <SDL2WAssetsProvider
          sprites={sprites}
          animations={animations}
          pictures={pictures}
        >
          <App assetTypes={assetTypes} />
        </SDL2WAssetsProvider>
      </React.StrictMode>
    );
  } catch (error) {
    console.error('Failed to initialize app:', error);
    const container = document.getElementById('root');
    if (container) {
      container.innerHTML = `
        <div class="container">
          <div class="error">
            Failed to load application: ${
              error instanceof Error ? error.message : 'Unknown error'
            }
          </div>
        </div>
      `;
    }
  }
}

init();
