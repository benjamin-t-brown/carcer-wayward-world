import { createContext, useContext, useState, ReactNode } from 'react';
import {
  ItemTemplate,
  CharacterTemplate,
  TilesetTemplate,
  GameEvent,
  CarcerMapTemplate,
} from '../types/assets';

interface AssetsContextType {
  // Assets
  items: ItemTemplate[];
  characters: CharacterTemplate[];
  tilesets: TilesetTemplate[];
  gameEvents: GameEvent[];
  maps: CarcerMapTemplate[];
  
  // Loading state
  loading: boolean;
  error: string | null;
  
  // Update functions
  setItems: (items: ItemTemplate[]) => void;
  setCharacters: (characters: CharacterTemplate[]) => void;
  setTilesets: (tilesets: TilesetTemplate[]) => void;
  setGameEvents: (gameEvents: GameEvent[]) => void;
  setMaps: (maps: CarcerMapTemplate[]) => void;
  
  // Save functions
  saveItems: (items: ItemTemplate[]) => Promise<void>;
  saveCharacters: (characters: CharacterTemplate[]) => Promise<void>;
  saveTilesets: (tilesets: TilesetTemplate[]) => Promise<void>;
  saveGameEvents: (gameEvents: GameEvent[]) => Promise<void>;
  saveMaps: (maps: CarcerMapTemplate[]) => Promise<void>;
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
  initialTilesets: TilesetTemplate[];
  initialGameEvents: GameEvent[];
  initialMaps: CarcerMapTemplate[];
}

async function saveItems(items: ItemTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/itemTemplates', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(items),
  });

  if (!response.ok) {
    throw new Error('Failed to save items');
  }
}

async function saveCharacters(characters: CharacterTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/characterTemplates', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(characters),
  });

  if (!response.ok) {
    throw new Error('Failed to save characters');
  }
}

async function saveTilesets(tilesets: TilesetTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/tilesetTemplates', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(tilesets),
  });

  if (!response.ok) {
    throw new Error('Failed to save tilesets');
  }
}

async function saveGameEvents(gameEvents: GameEvent[]): Promise<void> {
  const response = await fetch('/api/assets/specialEvents', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(gameEvents),
  });

  if (!response.ok) {
    throw new Error('Failed to save game events');
  }
}

async function saveMaps(maps: CarcerMapTemplate[]): Promise<void> {
  const response = await fetch('/api/assets/maps', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(maps),
  });

  if (!response.ok) {
    throw new Error('Failed to save maps');
  }
}

export function AssetsProvider({
  children,
  initialItems,
  initialCharacters,
  initialTilesets,
  initialGameEvents,
  initialMaps,
}: AssetsProviderProps) {
  const [items, setItems] = useState<ItemTemplate[]>(initialItems);
  const [characters, setCharacters] = useState<CharacterTemplate[]>(initialCharacters);
  const [tilesets, setTilesets] = useState<TilesetTemplate[]>(initialTilesets);
  const [gameEvents, setGameEvents] = useState<GameEvent[]>(initialGameEvents);
  const [maps, setMaps] = useState<CarcerMapTemplate[]>(initialMaps);
  const [loading] = useState(false);
  const [error] = useState<string | null>(null);

  return (
    <AssetsContext.Provider
      value={{
        items,
        characters,
        tilesets,
        gameEvents,
        maps,
        loading,
        error,
        setItems,
        setCharacters,
        setTilesets,
        setGameEvents,
        setMaps,
        saveItems,
        saveCharacters,
        saveTilesets,
        saveGameEvents,
        saveMaps,
      }}
    >
      {children}
    </AssetsContext.Provider>
  );
}

