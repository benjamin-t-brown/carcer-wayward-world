import { createContext, useContext, useState, ReactNode } from 'react';
import { useSDL2WAssets } from './SDL2WAssetsContext';
import { normalizeAll } from '../utils/assetNormalizers';
import {
  ItemTemplate,
  CharacterTemplate,
  TilesetTemplate,
  GameEvent,
  CarcerMapTemplate,
  MapGridTemplate,
  FeatTemplate,
  QuestTemplate,
} from '../types/assets';
import { AbilityTemplate, StatusEffectTemplate } from '../types/ability';
import { SpellTemplate } from '../types/spell';
import { ASSET_TYPES, AssetId } from '../../shared/assetRegistry';

interface AssetsContextType {
  items: ItemTemplate[];
  characters: CharacterTemplate[];
  abilities: AbilityTemplate[];
  spells: SpellTemplate[];
  statusEffects: StatusEffectTemplate[];
  feats: FeatTemplate[];
  quests: QuestTemplate[];
  tilesets: TilesetTemplate[];
  gameEvents: GameEvent[];
  maps: CarcerMapTemplate[];
  mapGrids: MapGridTemplate[];
  loading: boolean;
  error: string | null;
  setItems: (items: ItemTemplate[]) => void;
  setCharacters: (characters: CharacterTemplate[]) => void;
  setAbilities: (abilities: AbilityTemplate[]) => void;
  setSpells: (spells: SpellTemplate[]) => void;
  setStatusEffects: (statusEffects: StatusEffectTemplate[]) => void;
  setFeats: (feats: FeatTemplate[]) => void;
  setQuests: (quests: QuestTemplate[]) => void;
  setTilesets: (tilesets: TilesetTemplate[]) => void;
  setGameEvents: (gameEvents: GameEvent[]) => void;
  setMaps: (maps: CarcerMapTemplate[]) => void;
  setMapGrids: (mapGrids: MapGridTemplate[]) => void;
  saveItems: (items: ItemTemplate[]) => Promise<void>;
  saveCharacters: (characters: CharacterTemplate[]) => Promise<void>;
  saveAbilities: (abilities: AbilityTemplate[]) => Promise<void>;
  saveSpells: (spells: SpellTemplate[]) => Promise<void>;
  saveStatusEffects: (statusEffects: StatusEffectTemplate[]) => Promise<void>;
  saveFeats: (feats: FeatTemplate[]) => Promise<void>;
  saveQuests: (quests: QuestTemplate[]) => Promise<void>;
  saveTilesets: (tilesets: TilesetTemplate[]) => Promise<void>;
  saveGameEvents: (gameEvents: GameEvent[]) => Promise<void>;
  saveMaps: (maps: CarcerMapTemplate[]) => Promise<void>;
  saveMapGrids: (mapGrids: MapGridTemplate[]) => Promise<void>;
  /** Refetch every JSON asset list except the one this page is editing. */
  reloadOtherAssets: (except: AssetId) => Promise<void>;
}

const AssetsContext = createContext<AssetsContextType | undefined>(undefined);

export function useAssets() {
  const context = useContext(AssetsContext);
  if (!context) {
    throw new Error('useAssets must be used within an AssetsProvider');
  }
  return context;
}

interface AssetsProviderProps {
  children: ReactNode;
  initialItems: ItemTemplate[];
  initialCharacters: CharacterTemplate[];
  initialAbilities: AbilityTemplate[];
  initialSpells: SpellTemplate[];
  initialStatusEffects: StatusEffectTemplate[];
  initialFeats: FeatTemplate[];
  initialQuests: QuestTemplate[];
  initialTilesets: TilesetTemplate[];
  initialGameEvents: GameEvent[];
  initialMaps: CarcerMapTemplate[];
  initialMapGrids: MapGridTemplate[];
}

async function saveAsset(id: AssetId, data: unknown): Promise<void> {
  const response = await fetch(`/api/assets/${id}`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data),
  });
  if (!response.ok) {
    throw new Error(`Failed to save ${id}`);
  }
}

const saveItems = (items: ItemTemplate[]) => saveAsset('itemTemplates', items);
const saveCharacters = (characters: CharacterTemplate[]) =>
  saveAsset('characterTemplates', characters);
const saveAbilities = (abilities: AbilityTemplate[]) =>
  saveAsset('abilityTemplates', abilities);
const saveSpells = (spells: SpellTemplate[]) => saveAsset('spellTemplates', spells);
const saveStatusEffects = (statusEffects: StatusEffectTemplate[]) =>
  saveAsset('statusEffectTemplates', statusEffects);
const saveFeats = (feats: FeatTemplate[]) => saveAsset('featTemplates', feats);
const saveQuests = (quests: QuestTemplate[]) => saveAsset('questTemplates', quests);
const saveTilesets = (tilesets: TilesetTemplate[]) =>
  saveAsset('tilesetTemplates', tilesets);
const saveGameEvents = (gameEvents: GameEvent[]) =>
  saveAsset('specialEvents', gameEvents);
const saveMaps = (maps: CarcerMapTemplate[]) => saveAsset('maps', maps);
const saveMapGrids = (mapGrids: MapGridTemplate[]) =>
  saveAsset('mapGrids', mapGrids);

async function fetchAssetList(id: AssetId): Promise<unknown[]> {
  const response = await fetch(`/api/assets/${id}`);
  if (!response.ok) {
    throw new Error(`Failed to load ${id}`);
  }
  return response.json();
}

export function AssetsProvider({
  children,
  initialItems,
  initialCharacters,
  initialAbilities,
  initialSpells,
  initialStatusEffects,
  initialFeats,
  initialQuests,
  initialTilesets,
  initialGameEvents,
  initialMaps,
  initialMapGrids,
}: AssetsProviderProps) {
  const [items, setItems] = useState<ItemTemplate[]>(initialItems);
  const [characters, setCharacters] = useState<CharacterTemplate[]>(initialCharacters);
  const [abilities, setAbilities] = useState<AbilityTemplate[]>(initialAbilities);
  const [spells, setSpells] = useState<SpellTemplate[]>(initialSpells);
  const [statusEffects, setStatusEffects] =
    useState<StatusEffectTemplate[]>(initialStatusEffects);
  const [feats, setFeats] = useState<FeatTemplate[]>(initialFeats);
  const [quests, setQuests] = useState<QuestTemplate[]>(initialQuests);
  const [tilesets, setTilesets] = useState<TilesetTemplate[]>(initialTilesets);
  const [gameEvents, setGameEvents] = useState<GameEvent[]>(initialGameEvents);
  const [maps, setMaps] = useState<CarcerMapTemplate[]>(initialMaps);
  const [mapGrids, setMapGrids] = useState<MapGridTemplate[]>(initialMapGrids);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const { animationMap, soundMap } = useSDL2WAssets();

  const currentById: Record<AssetId, unknown[]> = {
    itemTemplates: items,
    characterTemplates: characters,
    abilityTemplates: abilities,
    spellTemplates: spells,
    statusEffectTemplates: statusEffects,
    featTemplates: feats,
    questTemplates: quests,
    tilesetTemplates: tilesets,
    specialEvents: gameEvents,
    maps,
    mapGrids,
  };

  const reloadOtherAssets = async (except: AssetId) => {
    setLoading(true);
    setError(null);
    try {
      const rawByType: Partial<Record<AssetId, unknown[]>> = {};
      await Promise.all(
        ASSET_TYPES.map(async (assetType) => {
          rawByType[assetType.id] =
            assetType.id === except
              ? currentById[assetType.id]
              : await fetchAssetList(assetType.id);
        }),
      );
      const normalized = normalizeAll(
        rawByType,
        { animationMap, soundMap },
        ASSET_TYPES,
      );
      if (except !== 'itemTemplates') setItems(normalized.itemTemplates as ItemTemplate[]);
      if (except !== 'characterTemplates') {
        setCharacters(normalized.characterTemplates as CharacterTemplate[]);
      }
      if (except !== 'abilityTemplates') {
        setAbilities(normalized.abilityTemplates as AbilityTemplate[]);
      }
      if (except !== 'spellTemplates') setSpells(normalized.spellTemplates as SpellTemplate[]);
      if (except !== 'statusEffectTemplates') {
        setStatusEffects(normalized.statusEffectTemplates as StatusEffectTemplate[]);
      }
      if (except !== 'featTemplates') setFeats(normalized.featTemplates as FeatTemplate[]);
      if (except !== 'questTemplates') setQuests(normalized.questTemplates as QuestTemplate[]);
      if (except !== 'tilesetTemplates') {
        setTilesets(normalized.tilesetTemplates as TilesetTemplate[]);
      }
      if (except !== 'specialEvents') setGameEvents(normalized.specialEvents as GameEvent[]);
      if (except !== 'maps') setMaps(normalized.maps as CarcerMapTemplate[]);
      if (except !== 'mapGrids') setMapGrids(normalized.mapGrids as MapGridTemplate[]);
    } catch (err) {
      const message = err instanceof Error ? err.message : 'Failed to reload assets';
      setError(message);
      throw err;
    } finally {
      setLoading(false);
    }
  };

  return (
    <AssetsContext.Provider
      value={{
        items,
        characters,
        abilities,
        spells,
        statusEffects,
        feats,
        quests,
        tilesets,
        gameEvents,
        maps,
        mapGrids,
        loading,
        error,
        setItems,
        setCharacters,
        setAbilities,
        setSpells,
        setStatusEffects,
        setFeats,
        setQuests,
        setTilesets,
        setGameEvents,
        setMaps,
        setMapGrids,
        saveItems,
        saveCharacters,
        saveAbilities,
        saveSpells,
        saveStatusEffects,
        saveFeats,
        saveQuests,
        saveTilesets,
        saveGameEvents,
        saveMaps,
        saveMapGrids,
        reloadOtherAssets,
      }}
    >
      {children}
    </AssetsContext.Provider>
  );
}
