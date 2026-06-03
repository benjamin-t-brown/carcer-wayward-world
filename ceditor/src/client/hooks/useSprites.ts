import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';

/**
 * Access SDL2W sprites and animations loaded at app startup.
 * Prefer useSDL2WAssets() directly when you also need spriteMap / animationMap.
 */
export function useSprites() {
  const { sprites, animations, spriteMap, animationMap, sounds, soundMap } =
    useSDL2WAssets();
  return {
    sprites,
    animations,
    spriteMap,
    animationMap,
    sounds,
    soundMap,
    loading: false,
    error: null as string | null,
  };
}
