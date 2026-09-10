import assert from 'node:assert/strict';
import test from 'node:test';

import {
  createMediaCatalog,
  parseAssetDefinitions,
  parseSoundVolume,
  resolveSoundPath,
} from '../../src/core/media/assetFileParser.js';

test('sound paths use the selected container', () => {
  assert.equal(
    resolveSoundPath('assets/snd/test.wav', 'ogg'),
    'assets/snd/test.ogg',
  );
  assert.equal(
    resolveSoundPath('assets/snd/test', 'wav'),
    'assets/snd/test.wav',
  );
  assert.equal(parseSoundVolume('0.25'), 0.25);
  assert.equal(parseSoundVolume('2'), 1);
  assert.equal(parseSoundVolume('-1'), 0);
  assert.equal(parseSoundVolume('attribution'), null);
});

test('asset definitions parse pictures, sprites, animations, and sounds', () => {
  const parsed = parseAssetDefinitions(
    `
      # comment
      Pic,terrain,assets/img/terrain.png
      Sprites,terrain,2,28,32
      Sound,step,assets/snd/step.wav,0.4,Example Author
      Anim,water,loop
      terrain_0 3
      terrain_1,4
      EndAnim
    `,
    'ogg',
  );

  assert.deepEqual(parsed.pictures, {
    terrain: 'assets/img/terrain.png',
  });
  assert.deepEqual(
    parsed.sprites.map(({ name, index, width, height }) => ({
      name,
      index,
      width,
      height,
    })),
    [
      { name: 'terrain_0', index: 0, width: 28, height: 32 },
      { name: 'terrain_1', index: 1, width: 28, height: 32 },
    ],
  );
  assert.deepEqual(parsed.sounds, [
    { name: 'step', path: 'assets/snd/step.ogg', volume: 0.4 },
  ]);
  assert.deepEqual(parsed.animations, [
    {
      name: 'water',
      loop: true,
      frames: [
        { spriteName: 'terrain_0', frames: 3 },
        { spriteName: 'terrain_1', frames: 4 },
      ],
    },
  ]);
});

test('catalog aggregation is deterministic and creates lookup indexes', () => {
  const catalog = createMediaCatalog({
    'assets.z.txt': 'Sound,z,assets/snd/z',
    'assets.a.txt': 'Pic,a,assets/img/a.png\nSprites,a,1,16,16',
  });

  assert.equal(catalog.spriteByName.get('a_0')?.pictureAlias, 'a');
  assert.equal(catalog.soundByName.get('z')?.path, 'assets/snd/z.wav');
  assert.equal(catalog.animations.length, 0);
});
