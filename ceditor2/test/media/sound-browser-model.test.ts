import assert from 'node:assert/strict';
import test from 'node:test';

import { matchesSoundSearch } from '../../src/apps/sounds/editorModel.js';
import { mediaAssetUrl } from '../../src/core/media/index.js';

const sound = {
  name: 'hit_metal3',
  path: 'assets/snd/hit_metal3.wav',
  volume: 1,
};

test('sound search covers names and source paths case-insensitively', () => {
  assert.equal(matchesSoundSearch(sound, 'METAL'), true);
  assert.equal(matchesSoundSearch(sound, 'snd/hit'), true);
  assert.equal(matchesSoundSearch(sound, 'music'), false);
});

test('media URLs strip the runtime assets prefix for the editor route', () => {
  assert.equal(
    mediaAssetUrl('./assets/snd/hit_metal3.wav'),
    '/game-assets/snd/hit_metal3.wav',
  );
});
