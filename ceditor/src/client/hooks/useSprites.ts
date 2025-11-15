import { useState, useEffect } from 'react';
import { Sprite, Animation } from '../types/assets';

interface SpritesData {
  sprites: Sprite[];
  animations: Animation[];
  loading: boolean;
  error: string | null;
}

export function useSprites(): SpritesData {
  const [sprites, setSprites] = useState<Sprite[]>([]);
  const [animations, setAnimations] = useState<Animation[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    async function loadSprites() {
      try {
        const response = await fetch('/api/assets/sprites');
        if (!response.ok) {
          throw new Error('Failed to load sprites');
        }
        const data = await response.json();
        setSprites(data.sprites || []);
        setAnimations(data.animations || []);
        setLoading(false);
      } catch (err) {
        setError(err instanceof Error ? err.message : 'Unknown error');
        setLoading(false);
      }
    }

    loadSprites();
  }, []);

  return { sprites, animations, loading, error };
}

