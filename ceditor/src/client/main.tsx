import React from 'react';
import { createRoot } from 'react-dom/client';
import App from './App';
import {
  Animation,
  loadSpritesAndAnimations,
  Sound,
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
  MapGridTemplate,
  FeatTemplate,
} from './types/assets';
import { AbilityTemplate, StatusEffectTemplate } from './types/ability';
import { SpellTemplate } from './types/spell';
import { ASSET_TYPES, AssetId } from '../shared/assetRegistry';
import { normalizeAll } from './utils/assetNormalizers';

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

async function fetchAssetList(id: string): Promise<any[]> {
  const response = await fetch(`/api/assets/${id}`);
  if (!response.ok) {
    throw new Error(`Failed to load ${id}`);
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
  spriteMap: Record<string, Sprite>;
  animations: Animation[];
  animationMap: Record<string, Animation>;
  sounds: Sound[];
  soundMap: Record<string, Sound>;
  pictures: Record<string, string>;
  items: ItemTemplate[];
  characters: CharacterTemplate[];
  abilities: AbilityTemplate[];
  spells: SpellTemplate[];
  statusEffects: StatusEffectTemplate[];
  feats: FeatTemplate[];
  tilesets: TilesetTemplate[];
  gameEvents: GameEvent[];
  maps: CarcerMapTemplate[];
  mapGrids: MapGridTemplate[];
}> {
  const assetTypes = await loadAssetTypes();
  const sdl2wAssetFiles = await loadSDL2WAssetFiles();
  const { sprites, animations, sounds, pictures } = await loadSpritesAndAnimations(
    sdl2wAssetFiles
  );
  for (const sprite of sprites) {
    const drawable = await getDrawable(sprite);
    if (!drawable) {
      throw new Error('Drawable not found');
    }
  }

  // Create sprite/animation maps for O(1) lookup by name
  const spriteMap: Record<string, Sprite> = {};
  for (const sprite of sprites) {
    spriteMap[sprite.name] = sprite;
  }

  const animationMap: Record<string, Animation> = {};
  for (const animation of animations) {
    animationMap[animation.name] = animation;
  }

  const soundMap: Record<string, Sound> = {};
  for (const sound of sounds) {
    soundMap[sound.name] = sound;
  }

  // Load every asset list in parallel, then normalize (dependents in a 2nd pass).
  const rawByType: Partial<Record<AssetId, any[]>> = {};
  await Promise.all(
    ASSET_TYPES.map(async (t) => {
      rawByType[t.id] = await fetchAssetList(t.id);
    })
  );
  const normalized = normalizeAll(rawByType, { animationMap, soundMap }, ASSET_TYPES);

  const items = normalized.itemTemplates as ItemTemplate[];
  const characters = normalized.characterTemplates as CharacterTemplate[];
  const abilities = normalized.abilityTemplates as AbilityTemplate[];
  const spells = normalized.spellTemplates as SpellTemplate[];
  const statusEffects = normalized.statusEffectTemplates as StatusEffectTemplate[];
  const feats = normalized.featTemplates as FeatTemplate[];
  const tilesets = normalized.tilesetTemplates as TilesetTemplate[];
  const gameEvents = normalized.specialEvents as GameEvent[];
  const maps = normalized.maps as CarcerMapTemplate[];
  const mapGrids = normalized.mapGrids as MapGridTemplate[];

  console.log('loaded', {
    assetTypes,
    sprites,
    animations,
    animationMap,
    sounds,
    soundMap,
    pictures,
    items,
    characters,
    abilities,
    spells,
    statusEffects,
    feats,
    tilesets,
    gameEvents,
    maps,
    mapGrids,
  });
  return {
    assetTypes,
    sprites,
    spriteMap,
    animations,
    animationMap,
    sounds,
    soundMap,
    pictures,
    items,
    characters,
    abilities,
    spells,
    statusEffects,
    feats,
    tilesets,
    gameEvents,
    maps,
    mapGrids,
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
      animationMap,
      sounds,
      soundMap,
      pictures,
      items,
      characters,
      abilities,
      spells,
      statusEffects,
      feats,
      tilesets,
      gameEvents,
      maps,
      mapGrids,
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
          animationMap={animationMap}
          sounds={sounds}
          soundMap={soundMap}
          pictures={pictures}
        >
          <AssetsProvider
            initialItems={items}
            initialCharacters={characters}
            initialAbilities={abilities}
            initialSpells={spells}
            initialStatusEffects={statusEffects}
            initialFeats={feats}
            initialTilesets={tilesets}
            initialGameEvents={gameEvents}
            initialMaps={maps}
            initialMapGrids={mapGrids}
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
