import type { DatabaseTransport } from '../../shared/databaseContract';
import { ASSET_TYPES } from '../../shared/assetRegistry';
import { DatabaseClient, DatabaseSession } from '../database';
import type { AbilityTemplate, StatusEffectTemplate } from '../types/ability';
import type {
  CarcerMapTemplate,
  CharacterTemplate,
  FeatTemplate,
  GameEvent,
  ItemTemplate,
  MapGridTemplate,
  TilesetTemplate,
} from '../types/assets';
import type { SpellTemplate } from '../types/spell';
import {
  loadSpritesAndAnimations,
  type Animation,
  type Sound,
  type Sprite,
} from '../utils/assetLoader';
import { normalizeAll } from '../utils/assetNormalizers';

export interface EditorBootstrapData {
  databaseSession: DatabaseSession;
  assetTypes: (typeof ASSET_TYPES)[number][];
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
}

export interface LoadEditorDataOptions {
  /** Test seam; production creates one DatabaseClient for one database GET. */
  databaseTransport?: DatabaseTransport;
  fetch?: typeof globalThis.fetch;
}

export async function loadEditorData(
  options: LoadEditorDataOptions = {},
): Promise<EditorBootstrapData> {
  const fetchImplementation =
    options.fetch ?? globalThis.fetch?.bind(globalThis);
  if (!fetchImplementation) {
    throw new Error('The Fetch API is not available');
  }

  const databaseTransport =
    options.databaseTransport ??
    new DatabaseClient({ fetch: fetchImplementation });
  const [databaseSession, sdl2wAssetFiles] = await Promise.all([
    DatabaseSession.load(databaseTransport),
    loadSDL2WAssetFiles(fetchImplementation),
  ]);
  const { sprites, animations, sounds, pictures } =
    await loadSpritesAndAnimations(sdl2wAssetFiles);

  const spriteMap = indexByName(sprites);
  const animationMap = indexByName(animations);
  const soundMap = indexByName(sounds);

  // Normalizers may migrate their input. Give them a defensive copy while the
  // DatabaseSession retains its raw loaded baseline and revision.
  const normalized = normalizeAll(
    databaseSession.snapshot(),
    { animationMap, soundMap },
    ASSET_TYPES,
  );

  return {
    databaseSession,
    assetTypes: [...ASSET_TYPES],
    sprites,
    spriteMap,
    animations,
    animationMap,
    sounds,
    soundMap,
    pictures,
    items: normalized.itemTemplates as ItemTemplate[],
    characters: normalized.characterTemplates as CharacterTemplate[],
    abilities: normalized.abilityTemplates as AbilityTemplate[],
    spells: normalized.spellTemplates as SpellTemplate[],
    statusEffects: normalized.statusEffectTemplates as StatusEffectTemplate[],
    // Feats have editor prototypes but no managed database file or route.
    feats: [],
    tilesets: normalized.tilesetTemplates as TilesetTemplate[],
    gameEvents: normalized.specialEvents as GameEvent[],
    maps: normalized.maps as CarcerMapTemplate[],
    mapGrids: normalized.mapGrids as MapGridTemplate[],
  };
}

function indexByName<T extends { name: string }>(
  records: readonly T[],
): Record<string, T> {
  const result: Record<string, T> = {};
  for (const record of records) {
    result[record.name] = record;
  }
  return result;
}

async function loadSDL2WAssetFiles(
  fetchImplementation: typeof globalThis.fetch,
): Promise<Record<string, string>> {
  const response = await fetchImplementation('/api/sdl2w-assets');
  if (!response.ok) {
    throw new Error('Failed to load SDL2W assets');
  }

  const value: unknown = await response.json();
  if (
    typeof value !== 'object' ||
    value === null ||
    Array.isArray(value) ||
    !Object.values(value).every((content) => typeof content === 'string')
  ) {
    throw new Error('SDL2W asset response is invalid');
  }
  return value as Record<string, string>;
}
