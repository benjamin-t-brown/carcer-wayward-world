import assert from 'node:assert/strict';
import test from 'node:test';

import {
  MapRenderDocument,
  parseTilesets,
} from '../../src/apps/maps/MapRenderDocument.js';
import type { JsonObject } from '../../src/core/database/index.js';
import { MapDocument } from '../../src/core/domain/maps/index.js';
import type {
  MediaCatalog,
  SpriteDefinition,
} from '../../src/core/media/index.js';

test('render adapter pre-indexes sprites and treats absent layers as blank', () => {
  const document = MapDocument.from({
    name: 'render-map',
    width: 1,
    height: 1,
    tilesets: ['', 'terrain0'],
    layers: [0],
    tiles: { '0': [1, 7] },
  });
  const sprite: SpriteDefinition = {
    name: 'terrain_7',
    pictureAlias: 'terrain',
    picturePath: 'img/game/terrain.png',
    index: 7,
    width: 28,
    height: 32,
  };
  const catalog: MediaCatalog = {
    sprites: [sprite],
    animations: [],
    sounds: [],
    pictures: { terrain: sprite.picturePath },
    spriteByName: new Map([[sprite.name, sprite]]),
    animationByName: new Map(),
    soundByName: new Map(),
  };
  const tilesets = parseTilesets([
    {
      name: 'terrain0',
      spriteBase: 'terrain',
      tiles: [{ id: 7, description: 'Ground' }],
    },
  ]);
  const renderDocument = new MapRenderDocument(document);
  renderDocument.rebuildSprites(catalog, tilesets);

  assert.equal(renderDocument.spriteAt(0, 0), sprite);
  assert.equal(renderDocument.spriteAt(1, 0), null);
  assert.equal(renderDocument.spriteAt(0, 1), undefined);
});

test('tileset parsing ignores incomplete records without normalizing input', () => {
  const input: JsonObject = {
    name: 'terrain0',
    spriteBase: 'terrain',
    tiles: [{ id: 1, description: 'Grass', future: true }, { id: 'bad' }],
    future: true,
  };

  assert.deepEqual(parseTilesets([input]).get('terrain0'), {
    name: 'terrain0',
    spriteBase: 'terrain',
    tiles: [{ id: 1, description: 'Grass' }],
  });
  assert.equal(input.future, true);
});
