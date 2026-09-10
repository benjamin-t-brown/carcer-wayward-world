import { fileURLToPath, URL } from 'node:url';

import { defineConfig } from 'vite';

function fromRoot(path: string): string {
  return fileURLToPath(new URL(path, import.meta.url));
}

export default defineConfig({
  server: {
    host: '127.0.0.1',
    port: 3000,
    proxy: {
      '/api': 'http://127.0.0.1:3001',
      '/game-assets': 'http://127.0.0.1:3001',
    },
  },
  preview: {
    host: '127.0.0.1',
    port: 3000,
    proxy: {
      '/api': 'http://127.0.0.1:3001',
      '/game-assets': 'http://127.0.0.1:3001',
    },
  },
  build: {
    outDir: 'dist',
    emptyOutDir: true,
    rollupOptions: {
      input: {
        home: fromRoot('./index.html'),
        items: fromRoot('./pages/items/index.html'),
        abilities: fromRoot('./pages/abilities/index.html'),
        spells: fromRoot('./pages/spells/index.html'),
        statusEffects: fromRoot('./pages/status-effects/index.html'),
        characters: fromRoot('./pages/characters/index.html'),
        tilesets: fromRoot('./pages/tilesets/index.html'),
        maps: fromRoot('./pages/maps/index.html'),
        mapGrids: fromRoot('./pages/map-grids/index.html'),
        events: fromRoot('./pages/events/index.html'),
        sounds: fromRoot('./pages/sounds/index.html'),
      },
    },
  },
});
