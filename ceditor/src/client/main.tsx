import React from 'react';
import { createRoot } from 'react-dom/client';
import App from './App';
import {
  Animation,
  loadSpritesAndAnimations,
  Sprite,
} from './utils/assetLoader';
import { SDL2WAssetsProvider } from './contexts/SDL2WAssetsContext';
import { AssetsProvider } from './contexts/AssetsContext';
import { getDrawable } from './utils/spriteUtils';
import {
  ItemTemplate,
  CharacterTemplate,
  TilesetTemplate,
  GameEvent,
  CarcerMapTemplate,
} from './types/assets';

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

async function loadItems(): Promise<ItemTemplate[]> {
  const response = await fetch('/api/assets/itemTemplates');
  if (!response.ok) {
    throw new Error('Failed to load items');
  }
  return response.json();
}

async function loadCharacters(): Promise<CharacterTemplate[]> {
  const response = await fetch('/api/assets/characterTemplates');
  if (!response.ok) {
    throw new Error('Failed to load characters');
  }
  return response.json();
}

async function loadTilesets(): Promise<TilesetTemplate[]> {
  const response = await fetch('/api/assets/tilesetTemplates');
  if (!response.ok) {
    throw new Error('Failed to load tilesets');
  }
  return response.json();
}

async function loadGameEvents(): Promise<GameEvent[]> {
  const response = await fetch('/api/assets/specialEvents');
  if (!response.ok) {
    throw new Error('Failed to load game events');
  }
  return response.json();
}

async function loadMaps(): Promise<CarcerMapTemplate[]> {
  const response = await fetch('/api/assets/maps');
  if (!response.ok) {
    throw new Error('Failed to load maps');
  }
  return response.json();
}

async function load(): Promise<{
  assetTypes: AssetType[];
  sprites: Sprite[];
  spriteMap: Record<string, Sprite>;
  animations: Animation[];
  pictures: Record<string, string>;
  items: ItemTemplate[];
  characters: CharacterTemplate[];
  tilesets: TilesetTemplate[];
  gameEvents: GameEvent[];
  maps: CarcerMapTemplate[];
}> {
  const assetTypes = await loadAssetTypes();
  const sdl2wAssetFiles = await loadSDL2WAssetFiles();
  const { sprites, animations, pictures } = await loadSpritesAndAnimations(
    sdl2wAssetFiles
  );
  for (const sprite of sprites) {
    const drawable = await getDrawable(sprite);
    if (!drawable) {
      throw new Error('Drawable not found');
    }
  }

  // Create sprite map for O(1) lookup by name
  const spriteMap: Record<string, Sprite> = {};
  for (const sprite of sprites) {
    spriteMap[sprite.name] = sprite;
  }

  // Load all assets in parallel
  const [items, characters, tilesets, gameEvents, maps] = await Promise.all([
    loadItems(),
    loadCharacters(),
    loadTilesets(),
    loadGameEvents(),
    loadMaps(),
  ]);

  // fixes problems from early design
  for (const map of maps) {
    for (const level in map.levels) {
      for (const tile of map.levels[level]) {
        if (!tile.markers) {
          tile.markers = [];
        }
      }
    }
  }

  console.log('loaded', {
    assetTypes,
    sprites,
    animations,
    pictures,
    items,
    characters,
    tilesets,
    gameEvents,
    maps,
  });
  return {
    assetTypes,
    sprites,
    spriteMap,
    animations,
    pictures,
    items,
    characters,
    tilesets,
    gameEvents,
    maps,
  };
}

async function init() {
  // Show a loading message in the body while assets are loading
  const container = document.getElementById('root');
  if (container) {
    container.innerHTML = `
      <div style="font-family: sans-serif; color: #bbb; text-align: center; margin-top: 60px;">
        Loading assets...
      </div>
    `;
  }
  try {
    const {
      assetTypes,
      sprites,
      spriteMap,
      animations,
      pictures,
      items,
      characters,
      tilesets,
      gameEvents,
      maps,
    } = await load();
    const container = document.getElementById('root');

    if (!container) {
      throw new Error('Root element not found');
    }
    container.innerHTML = '';

    const root = createRoot(container);
    root.render(
      <React.StrictMode>
        <SDL2WAssetsProvider
          sprites={sprites}
          spriteMap={spriteMap}
          animations={animations}
          pictures={pictures}
        >
          <AssetsProvider
            initialItems={items}
            initialCharacters={characters}
            initialTilesets={tilesets}
            initialGameEvents={gameEvents}
            initialMaps={maps}
          >
            <App assetTypes={assetTypes} />
          </AssetsProvider>
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

(window as any).debugTest = () => {
  const canvas = document.createElement('canvas');
  canvas.width = 1024;
  canvas.height = 1024;
  canvas.style.position = 'fixed';
  canvas.style.top = '0';
  canvas.style.left = '0';
  canvas.style.zIndex = '1000';
  const ctx = canvas.getContext('2d');
  if (!ctx) {
    throw new Error('Failed to get canvas context');
  }
  ctx.fillStyle = '#FFFFFF';
  ctx.fillRect(0, 0, 1024, 1024);
  document.body.appendChild(canvas);
  let i = 0;
  for (const [name, drawable] of Object.entries(
    (window as any).spriteCanvasCache
  )) {
    const c = drawable as HTMLCanvasElement;
    const x = (i % 16) * 64;
    const y = Math.floor(i / 16) * 64;
    ctx.drawImage(c, x, y);
    i++;
    ctx.font = '10px Arial';
    ctx.fillStyle = '#333';
    ctx.fillText(name, x, y);
  }
};
