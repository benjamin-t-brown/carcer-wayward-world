import { createContext, useContext, useState, ReactNode } from 'react';
import {
  ItemTemplate,
  CharacterTemplate,
  TilesetTemplate,
  GameEvent,
  CarcerMapTemplate,
  MapGridTemplate,
} from '../types/assets';
import { AbilityTemplate, StatusEffectTemplate } from '../types/ability';

interface AssetsContextType {
  items: ItemTemplate[];
  characters: CharacterTemplate[];
  abilities: AbilityTemplate[];
  statusEffects: StatusEffectTemplate[];
  tilesets: TilesetTemplate[];
  gameEvents: GameEvent[];
  maps: CarcerMapTemplate[];
  mapGrids: MapGridTemplate[];
  loading: boolean;
  error: string | null;
  setItems: (items: ItemTemplate[]) => void;
  setCharacters: (characters: CharacterTemplate[]) => void;
  setAbilities: (abilities: AbilityTemplate[]) => void;
  setStatusEffects: (statusEffects: StatusEffectTemplate[]) => void;
  setTilesets: (tilesets: TilesetTemplate[]) => void;
  setGameEvents: (gameEvents: GameEvent[]) => void;
  setMaps: (maps: CarcerMapTemplate[]) => void;
  setMapGrids: (mapGrids: MapGridTemplate[]) => void;
  saveItems: (items: ItemTemplate[]) => Promise<void>;
  saveCharacters: (characters: CharacterTemplate[]) => Promise<void>;
  saveAbilities: (abilities: AbilityTemplate[]) => Promise<void>;
  saveStatusEffects: (statusEffects: StatusEffectTemplate[]) => Promise<void>;
  saveTilesets: (tilesets: TilesetTemplate[]) => Promise<void>;
  saveGameEvents: (gameEvents: GameEvent[]) => Promise<void>;
  saveMaps: (maps: CarcerMapTemplate[]) => Promise<void>;
  saveMapGrids: (mapGrids: MapGridTemplate[]) => Promise<void>;
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
  initialStatusEffects: StatusEffectTemplate[];
  initialTilesets: TilesetTemplate[];
  initialGameEvents: GameEvent[];
  initialMaps: CarcerMapTemplate[];
  initialMapGrids: MapGridTemplate[];
}

async function saveItems(items: ItemTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/itemTemplates', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(items),
  });
  if (!response.ok) {
    throw new Error('Failed to save items');
  }
}

async function saveCharacters(characters: CharacterTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/characterTemplates', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(characters),
  });
  if (!response.ok) {
    throw new Error('Failed to save characters');
  }
}

async function saveAbilities(abilities: AbilityTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/abilityTemplates', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(abilities),
  });
  if (!response.ok) {
    throw new Error('Failed to save abilities');
  }
}

async function saveStatusEffects(statusEffects: StatusEffectTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/statusEffectTemplates', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(statusEffects),
  });
  if (!response.ok) {
    throw new Error('Failed to save status effects');
  }
}

async function saveTilesets(tilesets: TilesetTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/tilesetTemplates', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(tilesets),
  });
  if (!response.ok) {
    throw new Error('Failed to save tilesets');
  }
}

async function saveGameEvents(gameEvents: GameEvent[]): Promise<void> {
  const response = await fetch('/api/assets/specialEvents', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(gameEvents),
  });
  if (!response.ok) {
    throw new Error('Failed to save game events');
  }
}

async function saveMaps(maps: CarcerMapTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/maps', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(maps),
  });
  if (!response.ok) {
    throw new Error('Failed to save maps');
  }
}

async function saveMapGrids(mapGrids: MapGridTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/mapGrids', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(mapGrids),
  });
  if (!response.ok) {
    throw new Error('Failed to save map grids');
  }
}

export function AssetsProvider({
  children,
  initialItems,
  initialCharacters,
  initialAbilities,
  initialStatusEffects,
  initialTilesets,
  initialGameEvents,
  initialMaps,
  initialMapGrids,
}: AssetsProviderProps) {
  const [items, setItems] = useState<ItemTemplate[]>(initialItems);
  const [characters, setCharacters] = useState<CharacterTemplate[]>(initialCharacters);
  const [abilities, setAbilities] = useState<AbilityTemplate[]>(initialAbilities);
  const [statusEffects, setStatusEffects] =
    useState<StatusEffectTemplate[]>(initialStatusEffects);
  const [tilesets, setTilesets] = useState<TilesetTemplate[]>(initialTilesets);
  const [gameEvents, setGameEvents] = useState<GameEvent[]>(initialGameEvents);
  const [maps, setMaps] = useState<CarcerMapTemplate[]>(initialMaps);
  const [mapGrids, setMapGrids] = useState<MapGridTemplate[]>(initialMapGrids);
  const [loading] = useState(false);
  const [error] = useState<string | null>(null);

  return (
    <AssetsContext.Provider
      value={{
        items,
        characters,
        abilities,
        statusEffects,
        tilesets,
        gameEvents,
        maps,
        mapGrids,
        loading,
        error,
        setItems,
        setCharacters,
        setAbilities,
        setStatusEffects,
        setTilesets,
        setGameEvents,
        setMaps,
        setMapGrids,
        saveItems,
        saveCharacters,
        saveAbilities,
        saveStatusEffects,
        saveTilesets,
        saveGameEvents,
        saveMaps,
        saveMapGrids,
      }}
    >
      {children}
    </AssetsContext.Provider>
  );
}
