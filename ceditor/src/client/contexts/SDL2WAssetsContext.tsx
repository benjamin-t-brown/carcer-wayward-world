import { createContext, useContext, ReactNode } from 'react';
import { Sprite, Animation } from '../utils/assetLoader';

interface SDL2WAssetsContextValue {
  sprites: Sprite[];
  animations: Animation[];
  pictures: Record<string, string>;
}

const SDL2WAssetsContext = createContext<SDL2WAssetsContextValue | null>(null);

interface SDL2WAssetsProviderProps {
  children: ReactNode;
  sprites: Sprite[];
  animations: Animation[];
  pictures: Record<string, string>;
}

export function SDL2WAssetsProvider({
  children,
  sprites,
  animations,
  pictures,
}: SDL2WAssetsProviderProps) {
  return (
    <SDL2WAssetsContext.Provider value={{ sprites, animations, pictures }}>
      {children}
    </SDL2WAssetsContext.Provider>
  );
}

export function useSDL2WAssets(): SDL2WAssetsContextValue {
  const context = useContext(SDL2WAssetsContext);
  if (!context) {
    throw new Error('useSDL2WAssets must be used within SDL2WAssetsProvider');
  }
  return context;
}

