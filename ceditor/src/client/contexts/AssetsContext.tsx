import { createContext, useCallback, useContext, useRef, useState, ReactNode } from 'react';
import { useSDL2WAssets } from './SDL2WAssetsContext';
import { normalizeAll } from '../utils/assetNormalizers';
import { mergeAssetRecords } from '../utils/mergeAssetRecords';
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
import { ASSET_TYPES, ASSET_RECORD_KEY_BY_ID, AssetId } from '../../shared/assetRegistry';

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
  saveItems: (items: ItemTemplate[]) => Promise<ItemTemplate[]>;
  saveCharacters: (characters: CharacterTemplate[]) => Promise<CharacterTemplate[]>;
  saveAbilities: (abilities: AbilityTemplate[]) => Promise<AbilityTemplate[]>;
  saveSpells: (spells: SpellTemplate[]) => Promise<SpellTemplate[]>;
  saveStatusEffects: (
    statusEffects: StatusEffectTemplate[],
  ) => Promise<StatusEffectTemplate[]>;
  saveFeats: (feats: FeatTemplate[]) => Promise<FeatTemplate[]>;
  saveQuests: (quests: QuestTemplate[]) => Promise<QuestTemplate[]>;
  saveTilesets: (tilesets: TilesetTemplate[]) => Promise<TilesetTemplate[]>;
  saveGameEvents: (gameEvents: GameEvent[]) => Promise<GameEvent[]>;
  saveMaps: (maps: CarcerMapTemplate[]) => Promise<CarcerMapTemplate[]>;
  saveMapGrids: (mapGrids: MapGridTemplate[]) => Promise<MapGridTemplate[]>;
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
  /** File mtime, in ms, for each asset list when this tab loaded it. */
  initialAssetMtimes: Partial<Record<AssetId, number | null>>;
}

async function saveAsset(id: AssetId, data: unknown): Promise<number | null> {
  const response = await fetch(`/api/assets/${id}`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data),
    cache: 'no-store',
  });
  if (!response.ok) {
    throw new Error(`Failed to save ${id}`);
  }
  const body = await response.json().catch(() => null);
  if (!body || body.success !== true) {
    throw new Error(`Failed to save ${id}`);
  }
  return typeof body.mtimeMs === 'number' ? body.mtimeMs : null;
}

type DiskSnapshot = {
  mtimeMs: number | null;
  baseline: unknown[];
};

function cloneRecords(records: unknown[]): unknown[] {
  return JSON.parse(JSON.stringify(records)) as unknown[];
}

function parseMtimeHeader(header: string | null): number | null {
  if (header == null) {
    return null;
  }
  const parsed = Number(header);
  return Number.isFinite(parsed) ? parsed : null;
}

async function fetchAssetMtime(id: AssetId): Promise<number | null> {
  const response = await fetch(`/api/assets/${id}/mtime`, { cache: 'no-store' });
  if (!response.ok) {
    throw new Error(`Failed to stat ${id}`);
  }
  const body = await response.json();
  return typeof body.mtimeMs === 'number' ? body.mtimeMs : null;
}

async function fetchAssetList(
  id: AssetId,
): Promise<{ records: unknown[]; mtimeMs: number | null }> {
  const response = await fetch(`/api/assets/${id}`, { cache: 'no-store' });
  if (!response.ok) {
    throw new Error(`Failed to load ${id}`);
  }
  const records = await response.json();
  if (!Array.isArray(records)) {
    throw new Error(`Failed to load ${id}`);
  }
  return {
    records,
    mtimeMs: parseMtimeHeader(response.headers.get('X-Asset-Mtime')),
  };
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
  initialAssetMtimes,
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

  const diskById = useRef<Record<AssetId, DiskSnapshot>>({
    itemTemplates: {
      mtimeMs: initialAssetMtimes.itemTemplates ?? null,
      baseline: cloneRecords(initialItems),
    },
    characterTemplates: {
      mtimeMs: initialAssetMtimes.characterTemplates ?? null,
      baseline: cloneRecords(initialCharacters),
    },
    abilityTemplates: {
      mtimeMs: initialAssetMtimes.abilityTemplates ?? null,
      baseline: cloneRecords(initialAbilities),
    },
    spellTemplates: {
      mtimeMs: initialAssetMtimes.spellTemplates ?? null,
      baseline: cloneRecords(initialSpells),
    },
    statusEffectTemplates: {
      mtimeMs: initialAssetMtimes.statusEffectTemplates ?? null,
      baseline: cloneRecords(initialStatusEffects),
    },
    featTemplates: {
      mtimeMs: initialAssetMtimes.featTemplates ?? null,
      baseline: cloneRecords(initialFeats),
    },
    questTemplates: {
      mtimeMs: initialAssetMtimes.questTemplates ?? null,
      baseline: cloneRecords(initialQuests),
    },
    tilesetTemplates: {
      mtimeMs: initialAssetMtimes.tilesetTemplates ?? null,
      baseline: cloneRecords(initialTilesets),
    },
    specialEvents: {
      mtimeMs: initialAssetMtimes.specialEvents ?? null,
      baseline: cloneRecords(initialGameEvents),
    },
    maps: {
      mtimeMs: initialAssetMtimes.maps ?? null,
      baseline: cloneRecords(initialMaps),
    },
    mapGrids: {
      mtimeMs: initialAssetMtimes.mapGrids ?? null,
      baseline: cloneRecords(initialMapGrids),
    },
  });

  const applyList = useRef<Record<AssetId, (records: unknown[]) => void>>({
    itemTemplates: (records) => setItems(records as ItemTemplate[]),
    characterTemplates: (records) => setCharacters(records as CharacterTemplate[]),
    abilityTemplates: (records) => setAbilities(records as AbilityTemplate[]),
    spellTemplates: (records) => setSpells(records as SpellTemplate[]),
    statusEffectTemplates: (records) =>
      setStatusEffects(records as StatusEffectTemplate[]),
    featTemplates: (records) => setFeats(records as FeatTemplate[]),
    questTemplates: (records) => setQuests(records as QuestTemplate[]),
    tilesetTemplates: (records) => setTilesets(records as TilesetTemplate[]),
    specialEvents: (records) => setGameEvents(records as GameEvent[]),
    maps: (records) => setMaps(records as CarcerMapTemplate[]),
    mapGrids: (records) => setMapGrids(records as MapGridTemplate[]),
  });

  const rememberList = (id: AssetId, records: unknown[], mtimeMs: number | null) => {
    diskById.current[id] = {
      mtimeMs,
      baseline: cloneRecords(records),
    };
    applyList.current[id](records);
  };

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
      const loadedMtimes: Partial<Record<AssetId, number | null>> = {};
      await Promise.all(
        ASSET_TYPES.map(async (assetType) => {
          if (assetType.id === except) {
            rawByType[assetType.id] = currentById[assetType.id];
            return;
          }
          const loaded = await fetchAssetList(assetType.id);
          rawByType[assetType.id] = loaded.records;
          loadedMtimes[assetType.id] = loaded.mtimeMs;
        }),
      );
      const normalized = normalizeAll(
        rawByType,
        { animationMap, soundMap },
        ASSET_TYPES,
      );
      for (const assetType of ASSET_TYPES) {
        if (assetType.id === except) {
          continue;
        }
        rememberList(
          assetType.id,
          normalized[assetType.id] as unknown[],
          loadedMtimes[assetType.id] ?? null,
        );
      }
    } catch (err) {
      const message = err instanceof Error ? err.message : 'Failed to reload assets';
      setError(message);
      throw err;
    } finally {
      setLoading(false);
    }
  };

  const saveRecords = useCallback(async <T extends object,>(id: AssetId, incoming: T[]) => {
    const known = diskById.current[id];
    const diskMtime = await fetchAssetMtime(id);
    let toWrite = incoming;
    if (diskMtime != null && (known.mtimeMs == null || diskMtime > known.mtimeMs)) {
      const fresh = await fetchAssetList(id);
      toWrite = mergeAssetRecords(
        known.baseline as T[],
        incoming,
        fresh.records as T[],
        ASSET_RECORD_KEY_BY_ID[id],
      );
      const label = ASSET_TYPES.find((assetType) => assetType.id === id)?.name ?? id;
      console.log(`${label} file changed on disk; merged this tab's edits`);
    }
    const savedMtime = await saveAsset(id, toWrite);
    diskById.current[id] = {
      mtimeMs: savedMtime,
      baseline: cloneRecords(toWrite),
    };
    applyList.current[id](toWrite);
    return toWrite;
  }, []);

  const saveItems = useCallback(
    (next: ItemTemplate[]) => saveRecords('itemTemplates', next),
    [saveRecords],
  );
  const saveCharacters = useCallback(
    (next: CharacterTemplate[]) => saveRecords('characterTemplates', next),
    [saveRecords],
  );
  const saveAbilities = useCallback(
    (next: AbilityTemplate[]) => saveRecords('abilityTemplates', next),
    [saveRecords],
  );
  const saveSpells = useCallback(
    (next: SpellTemplate[]) => saveRecords('spellTemplates', next),
    [saveRecords],
  );
  const saveStatusEffects = useCallback(
    (next: StatusEffectTemplate[]) => saveRecords('statusEffectTemplates', next),
    [saveRecords],
  );
  const saveFeats = useCallback(
    (next: FeatTemplate[]) => saveRecords('featTemplates', next),
    [saveRecords],
  );
  const saveQuests = useCallback(
    (next: QuestTemplate[]) => saveRecords('questTemplates', next),
    [saveRecords],
  );
  const saveTilesets = useCallback(
    (next: TilesetTemplate[]) => saveRecords('tilesetTemplates', next),
    [saveRecords],
  );
  const saveGameEvents = useCallback(
    (next: GameEvent[]) => saveRecords('specialEvents', next),
    [saveRecords],
  );
  const saveMaps = useCallback(
    (next: CarcerMapTemplate[]) => saveRecords('maps', next),
    [saveRecords],
  );
  const saveMapGrids = useCallback(
    (next: MapGridTemplate[]) => saveRecords('mapGrids', next),
    [saveRecords],
  );

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
