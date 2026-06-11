import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

// Minimal round-trip check using the same logic as migrate-maps-flat.mjs
const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const mapsPath = join(root, 'src/assets/db/maps.json');
const maps = JSON.parse(readFileSync(mapsPath, 'utf8'));

for (const map of maps) {
  if (!Array.isArray(map.tilesets) || typeof map.tiles !== 'object') {
    throw new Error(`Map ${map.name} is not flat format`);
  }
  if (!Array.isArray(map.layers) || !map.layers.length) {
    throw new Error(`Map ${map.name} missing layers`);
  }
  for (const l of map.layers) {
    const graphics = map.tiles[String(l)];
    const expected = map.width * map.height * 2;
    if (!graphics || graphics.length !== expected) {
      throw new Error(
        `Map ${map.name} layer ${l} expected ${expected} graphics values, got ${graphics?.length ?? 0}`
      );
    }
  }
  for (const listName of [
    'characters',
    'items',
    'markers',
    'eventTriggers',
    'travelTriggers',
    'tileOverrides',
    'lightSources',
  ]) {
    for (const entry of map[listName] ?? []) {
      if (typeof entry.l !== 'number' || typeof entry.i !== 'number') {
        throw new Error(`Map ${map.name} ${listName} entry missing l/i`);
      }
    }
  }
}

console.log(`Validated ${maps.length} flat maps.`);
