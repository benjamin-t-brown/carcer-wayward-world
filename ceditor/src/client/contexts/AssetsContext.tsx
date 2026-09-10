import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useRef,
  useState,
  type ReactNode,
} from 'react';
import {
  ItemTemplate,
  CharacterTemplate,
  TilesetTemplate,
  GameEvent,
  CarcerMapTemplate,
  MapGridTemplate,
  FeatTemplate,
} from '../types/assets';
import { AbilityTemplate, StatusEffectTemplate } from '../types/ability';
import { SpellTemplate } from '../types/spell';
import type { AssetId } from '../../shared/assetRegistry';
import type { JsonArray } from '../../shared/databaseContract';
import { DatabaseSession } from '../database/DatabaseSession';

export interface DatabaseChanges {
  items?: ItemTemplate[];
  characters?: CharacterTemplate[];
  abilities?: AbilityTemplate[];
  spells?: SpellTemplate[];
  statusEffects?: StatusEffectTemplate[];
  tilesets?: TilesetTemplate[];
  gameEvents?: GameEvent[];
  maps?: CarcerMapTemplate[];
  mapGrids?: MapGridTemplate[];
}

interface AssetsContextType {
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
  loading: boolean;
  error: string | null;
  isSaving: boolean;
  isDirty: boolean;
  dirtyAssetIds: ReadonlySet<AssetId>;
  saveError: string | null;
  setItems: (items: ItemTemplate[]) => void;
  setCharacters: (characters: CharacterTemplate[]) => void;
  setAbilities: (abilities: AbilityTemplate[]) => void;
  setSpells: (spells: SpellTemplate[]) => void;
  setStatusEffects: (statusEffects: StatusEffectTemplate[]) => void;
  setFeats: (feats: FeatTemplate[]) => void;
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
  saveTilesets: (tilesets: TilesetTemplate[]) => Promise<void>;
  saveGameEvents: (gameEvents: GameEvent[]) => Promise<void>;
  saveMaps: (maps: CarcerMapTemplate[]) => Promise<void>;
  saveMapGrids: (mapGrids: MapGridTemplate[]) => Promise<void>;
  saveDatabaseChanges: (changes: DatabaseChanges) => Promise<void>;
  saveAll: () => Promise<void>;
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
  session: DatabaseSession;
  initialItems: ItemTemplate[];
  initialCharacters: CharacterTemplate[];
  initialAbilities: AbilityTemplate[];
  initialSpells: SpellTemplate[];
  initialStatusEffects: StatusEffectTemplate[];
  initialFeats: FeatTemplate[];
  initialTilesets: TilesetTemplate[];
  initialGameEvents: GameEvent[];
  initialMaps: CarcerMapTemplate[];
  initialMapGrids: MapGridTemplate[];
}

function useSessionCollection<T>(
  session: DatabaseSession,
  id: AssetId,
  initialRecords: T[],
  notifySessionChanged: () => void,
): [T[], (records: T[]) => void] {
  const [records, setRecords] = useState<T[]>(initialRecords);
  const replaceRecords = useCallback(
    (nextRecords: T[]) => {
      session.replaceCollection(id, nextRecords as unknown as JsonArray);
      setRecords(nextRecords);
      notifySessionChanged();
    },
    [id, notifySessionChanged, session],
  );

  return [records, replaceRecords];
}

export function AssetsProvider({
  children,
  session,
  initialItems,
  initialCharacters,
  initialAbilities,
  initialSpells,
  initialStatusEffects,
  initialFeats,
  initialTilesets,
  initialGameEvents,
  initialMaps,
  initialMapGrids,
}: AssetsProviderProps) {
  const [, setSessionVersion] = useState(0);
  const notifySessionChanged = useCallback(() => {
    setSessionVersion((version) => version + 1);
  }, []);
  const [items, setItems] = useSessionCollection(
    session,
    'itemTemplates',
    initialItems,
    notifySessionChanged,
  );
  const [characters, setCharacters] = useSessionCollection(
    session,
    'characterTemplates',
    initialCharacters,
    notifySessionChanged,
  );
  const [abilities, setAbilities] = useSessionCollection(
    session,
    'abilityTemplates',
    initialAbilities,
    notifySessionChanged,
  );
  const [spells, setSpells] = useSessionCollection(
    session,
    'spellTemplates',
    initialSpells,
    notifySessionChanged,
  );
  const [statusEffects, setStatusEffects] = useSessionCollection(
    session,
    'statusEffectTemplates',
    initialStatusEffects,
    notifySessionChanged,
  );
  const [feats, setFeats] = useState<FeatTemplate[]>(initialFeats);
  const [tilesets, setTilesets] = useSessionCollection(
    session,
    'tilesetTemplates',
    initialTilesets,
    notifySessionChanged,
  );
  const [gameEvents, setGameEvents] = useSessionCollection(
    session,
    'specialEvents',
    initialGameEvents,
    notifySessionChanged,
  );
  const [maps, setMaps] = useSessionCollection(
    session,
    'maps',
    initialMaps,
    notifySessionChanged,
  );
  const [mapGrids, setMapGrids] = useSessionCollection(
    session,
    'mapGrids',
    initialMapGrids,
    notifySessionChanged,
  );
  const [loading] = useState(false);
  const [error] = useState<string | null>(null);
  const [isSaving, setIsSaving] = useState(false);
  const [saveError, setSaveError] = useState<string | null>(null);
  const savingCount = useRef(0);

  useEffect(() => {
    const beforeUnload = (event: BeforeUnloadEvent) => {
      if (!session.isDirty) return;
      event.preventDefault();
      event.returnValue = '';
    };
    window.addEventListener('beforeunload', beforeUnload);
    return () => {
      window.removeEventListener('beforeunload', beforeUnload);
    };
  }, [session]);

  const saveSession = useCallback(async () => {
    savingCount.current += 1;
    setIsSaving(true);
    setSaveError(null);

    try {
      await session.saveAll();
      setSaveError(null);
    } catch (caughtError) {
      setSaveError(
        caughtError instanceof Error
          ? caughtError.message
          : 'Failed to save database',
      );
      throw caughtError;
    } finally {
      savingCount.current -= 1;
      if (savingCount.current === 0) {
        setIsSaving(false);
      }
      notifySessionChanged();
    }
  }, [notifySessionChanged, session]);

  const saveItems = useCallback(
    async (nextItems: ItemTemplate[]) => {
      setItems(nextItems);
      await saveSession();
    },
    [saveSession, setItems],
  );
  const saveCharacters = useCallback(
    async (nextCharacters: CharacterTemplate[]) => {
      setCharacters(nextCharacters);
      await saveSession();
    },
    [saveSession, setCharacters],
  );
  const saveAbilities = useCallback(
    async (nextAbilities: AbilityTemplate[]) => {
      setAbilities(nextAbilities);
      await saveSession();
    },
    [saveSession, setAbilities],
  );
  const saveSpells = useCallback(
    async (nextSpells: SpellTemplate[]) => {
      setSpells(nextSpells);
      await saveSession();
    },
    [saveSession, setSpells],
  );
  const saveStatusEffects = useCallback(
    async (nextStatusEffects: StatusEffectTemplate[]) => {
      setStatusEffects(nextStatusEffects);
      await saveSession();
    },
    [saveSession, setStatusEffects],
  );
  const saveFeats = useCallback(async (_nextFeats: FeatTemplate[]) => {
    throw new Error('Feat templates are not an active managed asset type');
  }, []);
  const saveTilesets = useCallback(
    async (nextTilesets: TilesetTemplate[]) => {
      setTilesets(nextTilesets);
      await saveSession();
    },
    [saveSession, setTilesets],
  );
  const saveGameEvents = useCallback(
    async (nextGameEvents: GameEvent[]) => {
      setGameEvents(nextGameEvents);
      await saveSession();
    },
    [saveSession, setGameEvents],
  );
  const saveMaps = useCallback(
    async (nextMaps: CarcerMapTemplate[]) => {
      setMaps(nextMaps);
      await saveSession();
    },
    [saveSession, setMaps],
  );
  const saveMapGrids = useCallback(
    async (nextMapGrids: MapGridTemplate[]) => {
      setMapGrids(nextMapGrids);
      await saveSession();
    },
    [saveSession, setMapGrids],
  );
  const saveDatabaseChanges = useCallback(
    async (changes: DatabaseChanges) => {
      if (changes.items !== undefined) setItems(changes.items);
      if (changes.characters !== undefined) setCharacters(changes.characters);
      if (changes.abilities !== undefined) setAbilities(changes.abilities);
      if (changes.spells !== undefined) setSpells(changes.spells);
      if (changes.statusEffects !== undefined)
        setStatusEffects(changes.statusEffects);
      if (changes.tilesets !== undefined) setTilesets(changes.tilesets);
      if (changes.gameEvents !== undefined) setGameEvents(changes.gameEvents);
      if (changes.maps !== undefined) setMaps(changes.maps);
      if (changes.mapGrids !== undefined) setMapGrids(changes.mapGrids);
      await saveSession();
    },
    [
      saveSession,
      setAbilities,
      setCharacters,
      setGameEvents,
      setItems,
      setMapGrids,
      setMaps,
      setSpells,
      setStatusEffects,
      setTilesets,
    ],
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
        tilesets,
        gameEvents,
        maps,
        mapGrids,
        loading,
        error,
        isSaving,
        isDirty: session.isDirty,
        dirtyAssetIds: session.dirtyAssetIds,
        saveError,
        setItems,
        setCharacters,
        setAbilities,
        setSpells,
        setStatusEffects,
        setFeats,
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
        saveTilesets,
        saveGameEvents,
        saveMaps,
        saveMapGrids,
        saveDatabaseChanges,
        saveAll: saveSession,
      }}
    >
      {children}
    </AssetsContext.Provider>
  );
}
