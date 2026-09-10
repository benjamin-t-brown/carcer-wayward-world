import assert from 'node:assert/strict';
import test from 'node:test';

import {
  createDefaultAbilityDepiction,
  sanitizeAbilityDepiction,
} from '../client/types/ability';
import type { Animation } from '../client/utils/assetLoader';
import {
  animationFrameIndexAtTime,
  filterMediaChoices,
  nextMediaRenderLimit,
} from '../client/utils/mediaPicker';

test('media filtering searches all supplied fields case-insensitively', () => {
  const choices = [
    { name: 'Fire Bolt', path: 'img/projectiles.png', sheet: 'magic' },
    { name: 'Guard', path: 'img/people.png', sheet: 'characters' },
  ];

  assert.deepEqual(
    filterMediaChoices(choices, 'PROJECT', (choice) => [
      choice.name,
      choice.path,
      choice.sheet,
    ]),
    [choices[0]],
  );
  assert.deepEqual(
    filterMediaChoices(choices, '  ', () => []),
    choices,
  );
});

test('incremental limits eventually expose every result without truncation', () => {
  let limit = 72;
  limit = nextMediaRenderLimit(limit, 260, 72);
  assert.equal(limit, 144);
  limit = nextMediaRenderLimit(limit, 260, 72);
  limit = nextMediaRenderLimit(limit, 260, 72);
  assert.equal(limit, 260);
});

test('a shared animation clock resolves looping and non-looping frames', () => {
  const looping: Animation = {
    name: 'pulse',
    loop: true,
    frames: [
      { spriteName: 'a', frames: 100 },
      { spriteName: 'b', frames: 200 },
    ],
  };
  const once: Animation = { ...looping, loop: false };

  assert.equal(animationFrameIndexAtTime(looping, 0), 0);
  assert.equal(animationFrameIndexAtTime(looping, 100), 1);
  assert.equal(animationFrameIndexAtTime(looping, 300), 0);
  assert.equal(animationFrameIndexAtTime(once, 10_000), 1);
});

test('unavailable legacy animation and sound values survive normalization', () => {
  const depiction = {
    ...createDefaultAbilityDepiction(),
    dmgAnim: 'legacy_animation',
    startSound: 'legacy_start_sound',
    dmgSound: 'legacy_damage_sound',
  };

  const normalized = sanitizeAbilityDepiction(depiction, {}, {});

  assert.equal(normalized.dmgAnim, 'legacy_animation');
  assert.equal(normalized.startSound, 'legacy_start_sound');
  assert.equal(normalized.dmgSound, 'legacy_damage_sound');
});
