import assert from 'node:assert/strict';
import { copyFile, mkdtemp, readFile, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import test from 'node:test';

import { ASSET_REGISTRY } from '../../src/core/database/assetRegistry.js';
import type { JsonObject } from '../../src/core/database/types.js';
import { validateDatabase } from '../../src/core/validation/index.js';
import { DEFAULT_DATABASE_PATH } from '../../src/server/app.js';
import {
  DatabaseRepository,
  serializeSnapshot,
} from '../../src/server/databaseRepository.js';

test('the complete checked-in database survives a golden serialization round trip', async () => {
  const repository = new DatabaseRepository(DEFAULT_DATABASE_PATH);
  const loaded = await repository.load();

  assert.equal(validateDatabase(loaded.assets).valid, true);
  for (const output of serializeSnapshot(loaded.assets)) {
    const definition = ASSET_REGISTRY.find(
      ({ fileName }) => fileName === output.fileName,
    )!;
    assert.deepEqual(JSON.parse(output.content), loaded.assets[definition.id]);
  }
});

test('a temporary real database supports a coherent multi-editor Save All and reload', async () => {
  const fixturePath = await mkdtemp(join(tmpdir(), 'ceditor2-real-db-'));
  try {
    await Promise.all(
      ASSET_REGISTRY.map(({ fileName }) =>
        copyFile(
          join(DEFAULT_DATABASE_PATH, fileName),
          join(fixturePath, fileName),
        ),
      ),
    );
    const beforeBytes = new Map(
      await Promise.all(
        ASSET_REGISTRY.map(
          async ({ fileName }) =>
            [
              fileName,
              await readFile(join(fixturePath, fileName), 'utf8'),
            ] as const,
        ),
      ),
    );
    const repository = new DatabaseRepository(fixturePath);
    const loaded = await repository.load();

    const noOp = await repository.save({
      baseRevision: loaded.revision,
      assets: loaded.assets,
    });
    assert.deepEqual(noOp.changedFiles, []);
    for (const { fileName } of ASSET_REGISTRY) {
      assert.equal(
        await readFile(join(fixturePath, fileName), 'utf8'),
        beforeBytes.get(fileName),
      );
    }

    const edited = structuredClone(loaded.assets);
    (edited.items[0] as JsonObject).description = 'CEditor2 fixture edit';
    (edited.maps[0] as JsonObject).label = 'CEditor2 fixture map';
    (edited.specialEvents[0] as JsonObject).title = 'CEditor2 fixture event';
    const saved = await repository.save({
      baseRevision: noOp.revision,
      assets: edited,
    });

    assert.deepEqual(saved.changedFiles, [
      'items.json',
      'maps.json',
      'special-events.json',
    ]);
    const reloaded = await repository.load();
    assert.equal(reloaded.revision, saved.revision);
    assert.deepEqual(reloaded.assets, edited);
    assert.equal(validateDatabase(reloaded.assets).valid, true);
  } finally {
    await rm(fixturePath, { recursive: true, force: true });
  }
});
