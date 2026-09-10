import assert from 'node:assert/strict';
import test from 'node:test';

import type { MediaCatalog } from '../../src/core/media/index.js';
import {
  animationSpriteAt,
  characterSpriteName,
  mediaChoiceName,
  mediaChoices,
} from '../../src/core/ui/mediaPickerModel.js';

const sprites = [
  {
    name: 'items_1',
    pictureAlias: 'items',
    picturePath: 'assets/img/items.png',
    index: 1,
    width: 16,
    height: 16,
  },
  {
    name: 'actors_0',
    pictureAlias: 'actors',
    picturePath: 'assets/img/actors.png',
    index: 0,
    width: 16,
    height: 24,
  },
];
const animations = [
  {
    name: 'spark',
    loop: true,
    frames: [
      { spriteName: 'items_1', frames: 2 },
      { spriteName: 'actors_0', frames: 1 },
    ],
  },
];
const sounds = [{ name: 'impact', path: 'assets/snd/impact.ogg', volume: 0.5 }];
const catalog: MediaCatalog = {
  sprites,
  animations,
  sounds,
  pictures: {
    actors: 'assets/img/actors.png',
    items: 'assets/img/items.png',
  },
  spriteByName: new Map(sprites.map((sprite) => [sprite.name, sprite])),
  animationByName: new Map(
    animations.map((animation) => [animation.name, animation]),
  ),
  soundByName: new Map(sounds.map((sound) => [sound.name, sound])),
};

test('media choices filter by kind, search text, and sprite sheet', () => {
  assert.deepEqual(mediaChoices(catalog, 'sprite').map(mediaChoiceName), [
    'actors_0',
    'items_1',
  ]);
  assert.deepEqual(
    mediaChoices(catalog, 'sprite', '', 'items').map(mediaChoiceName),
    ['items_1'],
  );
  assert.deepEqual(
    mediaChoices(catalog, 'picture', 'actors').map(mediaChoiceName),
    ['actors'],
  );
  assert.deepEqual(
    mediaChoices(catalog, 'sound', 'impact').map(mediaChoiceName),
    ['impact'],
  );
});

test('animation preview respects frame durations and loop behavior', () => {
  const loop = animations[0]!;
  assert.equal(animationSpriteAt(loop, 0), 'items_1');
  assert.equal(animationSpriteAt(loop, 199), 'items_1');
  assert.equal(animationSpriteAt(loop, 200), 'actors_0');
  assert.equal(animationSpriteAt(loop, 300), 'items_1');

  assert.equal(animationSpriteAt({ ...loop, loop: false }, 500), 'actors_0');
  assert.equal(animationSpriteAt({ ...loop, frames: [] }, 0), undefined);
});

test('character sprite names preserve string and numeric offsets', () => {
  assert.equal(characterSpriteName('actors', 4), 'actors_4');
  assert.equal(characterSpriteName('actors', 'merchant'), 'actors_merchant');
});
