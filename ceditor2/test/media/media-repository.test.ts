import assert from 'node:assert/strict';
import { mkdtemp, rm, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import test from 'node:test';

import { MediaSourceRepository } from '../../src/server/mediaRepository.js';

test('media repository returns only sorted SDL2W asset definition files', async () => {
  const directory = await mkdtemp(join(tmpdir(), 'ceditor2-media-'));
  try {
    await Promise.all([
      writeFile(join(directory, 'assets.z.txt'), 'Sound,z,z.wav', 'utf8'),
      writeFile(join(directory, 'assets.a.txt'), 'Pic,a,a.png', 'utf8'),
      writeFile(join(directory, 'translation.en.txt'), 'ignored', 'utf8'),
    ]);

    const sources = await new MediaSourceRepository(directory).load();

    assert.deepEqual(Object.keys(sources), ['assets.a.txt', 'assets.z.txt']);
    assert.equal(sources['assets.a.txt'], 'Pic,a,a.png');
  } finally {
    await rm(directory, { recursive: true, force: true });
  }
});
