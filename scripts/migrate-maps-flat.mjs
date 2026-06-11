import { readFileSync, writeFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const mapsPath = join(root, 'src/assets/db/maps.json');

function tileIndex(x, y, width) {
  return y * width + x;
}

function createEmptyTileGraphics(width, height) {
  const graphics = new Array(width * height * 2).fill(0);
  return graphics;
}

function getOrAddTilesetIndex(map, tilesetName) {
  const idx = map.tilesets.indexOf(tilesetName);
  if (idx >= 0) {
    return idx;
  }
  map.tilesets.push(tilesetName);
  return map.tilesets.length - 1;
}

function migrateLegacyMap(legacy) {
  const map = {
    name: legacy.name,
    label: legacy.label,
    type: legacy.type,
    width: legacy.width,
    height: legacy.height,
    spriteWidth: legacy.spriteWidth,
    spriteHeight: legacy.spriteHeight,
    tilesets: [''],
    layers: [],
    tiles: {},
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };

  const levelKeys = Object.keys(legacy.levels ?? {}).sort(
    (a, b) => parseInt(b, 10) - parseInt(a, 10)
  );
  map.layers = levelKeys.map((k) => parseInt(k, 10));

  for (const levelKey of levelKeys) {
    const l = parseInt(levelKey, 10);
    const legacyTiles = legacy.levels[levelKey] ?? [];
    const graphics = createEmptyTileGraphics(map.width, map.height);

    legacyTiles.forEach((tile, i) => {
      const tsIdx = getOrAddTilesetIndex(map, tile.tilesetName ?? '');
      graphics[i * 2] = tsIdx;
      graphics[i * 2 + 1] = tile.tileId ?? 0;

      for (const name of tile.characters ?? []) {
        if (name) {
          map.characters.push({ l, i, name });
        }
      }
      for (const item of tile.items ?? []) {
        map.items.push({
          l,
          i,
          name: item.name,
          quantity: item.quantity ?? 1,
        });
      }
      for (const name of tile.markers ?? []) {
        if (name) {
          map.markers.push({ l, i, name });
        }
      }
      if (tile.eventTrigger) {
        map.eventTriggers.push({ l, i, ...tile.eventTrigger });
      }
      if (tile.travelTrigger) {
        map.travelTriggers.push({ l, i, ...tile.travelTrigger });
      }
      if (tile.tileOverrides) {
        map.tileOverrides.push({ l, i, overrides: tile.tileOverrides });
      }
      if (tile.lightSource) {
        map.lightSources.push({ l, i, ...tile.lightSource });
      }
    });

    map.tiles[String(l)] = graphics;
  }

  if (!map.layers.length) {
    map.layers = [0];
    map.tiles['0'] = createEmptyTileGraphics(map.width, map.height);
  }

  return map;
}

function isLegacyMap(raw) {
  return raw.levels !== undefined && !Array.isArray(raw.tilesets);
}

const raw = JSON.parse(readFileSync(mapsPath, 'utf8'));
const migrated = raw.map((map) => (isLegacyMap(map) ? migrateLegacyMap(map) : map));
writeFileSync(mapsPath, JSON.stringify(migrated, null, 2) + '\n');
console.log(`Migrated ${migrated.length} maps to flat format.`);
