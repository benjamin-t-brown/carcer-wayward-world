import assert from 'node:assert/strict';
import {
  mkdtemp,
  readFile,
  rename,
  rm,
  stat,
  writeFile,
} from 'node:fs/promises';
import type { Server } from 'node:http';
import type { AddressInfo } from 'node:net';
import { join } from 'node:path';
import { tmpdir } from 'node:os';
import test from 'node:test';

import { ASSET_REGISTRY } from '../../src/core/database/assetRegistry.js';
import type {
  DatabaseEnvelope,
  DatabaseSnapshot,
  SaveDatabaseResponse,
} from '../../src/core/database/types.js';
import { createApp } from '../../src/server/app.js';
import { commitSaveTransaction } from '../../src/server/saveTransaction.js';

const LEGACY_TILES = '[{"legacy":true}]\n';

test('GET /api/database loads all managed arrays and returns a stable revision', async () => {
  await withFixture(async ({ baseUrl, snapshot }) => {
    const first = await fetch(`${baseUrl}/api/database`);
    assert.equal(first.status, 200);
    const firstBody = (await first.json()) as DatabaseEnvelope;

    assert.deepEqual(firstBody.assets, snapshot);
    assert.match(firstBody.revision, /^[a-f0-9]{64}$/);

    const second = await fetch(`${baseUrl}/api/database`);
    const secondBody = (await second.json()) as DatabaseEnvelope;
    assert.equal(secondBody.revision, firstBody.revision);
  });
});

test('PUT /api/database serializes and replaces only changed managed files', async () => {
  await withFixture(async ({ baseUrl, databasePath, snapshot }) => {
    const before = await readManagedContents(databasePath);
    const loaded = await loadDatabase(baseUrl);
    const next = structuredClone(snapshot);
    next.items.push({ name: 'NEW_TEST_ITEM' });

    const response = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: next,
    });
    assert.equal(response.status, 200);
    const body = (await response.json()) as SaveDatabaseResponse;
    assert.deepEqual(body.changedFiles, ['items.json']);
    assert.match(body.revision, /^[a-f0-9]{64}$/);
    assert.notEqual(body.revision, loaded.revision);

    const savedItems = await readFile(join(databasePath, 'items.json'), 'utf8');
    assert.equal(savedItems, `${JSON.stringify(next.items, null, 2)}\n`);

    const after = await readManagedContents(databasePath);
    for (const { fileName } of ASSET_REGISTRY) {
      if (fileName !== 'items.json') {
        assert.equal(after.get(fileName), before.get(fileName));
      }
    }
  });
});

test('PUT /api/database is a byte-preserving no-op for canonical content', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const loaded = await loadDatabase(baseUrl);
    const before = await readManagedContents(databasePath);
    const beforeStats = await statManagedFiles(databasePath);

    const response = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: loaded.assets,
    });
    assert.equal(response.status, 200);
    const body = (await response.json()) as SaveDatabaseResponse;
    assert.deepEqual(body.changedFiles, []);
    assert.equal(body.revision, loaded.revision);
    assert.deepEqual(await readManagedContents(databasePath), before);

    const afterStats = await statManagedFiles(databasePath);
    assert.deepEqual(afterStats, beforeStats);
  });
});

test('PUT /api/database rejects a stale base revision without writing', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const loaded = await loadDatabase(baseUrl);
    const originalItems = await readFile(
      join(databasePath, 'items.json'),
      'utf8',
    );
    await writeFile(
      join(databasePath, 'abilities.json'),
      `${JSON.stringify([{ externallyChanged: true }], null, 2)}\n`,
      'utf8',
    );

    const next = structuredClone(loaded.assets);
    next.items.push({ name: 'MUST_NOT_BE_WRITTEN' });
    const response = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: next,
    });

    assert.equal(response.status, 409);
    assert.deepEqual(await response.json(), {
      error: 'The database changed on disk. Reload before saving again.',
    });
    assert.equal(
      await readFile(join(databasePath, 'items.json'), 'utf8'),
      originalItems,
    );
  });
});

test('PUT /api/database rejects incomplete and non-array snapshots', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const loaded = await loadDatabase(baseUrl);
    const before = await readManagedContents(databasePath);
    const incomplete = structuredClone(loaded.assets) as Record<
      string,
      unknown
    >;
    delete incomplete.maps;

    const missingResponse = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: incomplete,
    });
    assert.equal(missingResponse.status, 400);

    const wrongType = structuredClone(loaded.assets) as Record<string, unknown>;
    wrongType.maps = {};
    const wrongTypeResponse = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: wrongType,
    });
    assert.equal(wrongTypeResponse.status, 400);

    assert.deepEqual(await readManagedContents(databasePath), before);
  });
});

test('successful Save All never touches unmanaged tiles.json', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const tilesPath = join(databasePath, 'tiles.json');
    assert.equal(await readFile(tilesPath, 'utf8'), LEGACY_TILES);

    const loaded = await loadDatabase(baseUrl);
    const next = structuredClone(loaded.assets);
    next.statusEffects.push({ name: 'NEW_STATUS' });
    const response = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: next,
    });

    assert.equal(response.status, 200);
    assert.equal(await readFile(tilesPath, 'utf8'), LEGACY_TILES);
  });
});

test('a failed multi-file replacement restores files already replaced', async () => {
  const databasePath = await mkdtemp(join(tmpdir(), 'ceditor2-rollback-'));
  const originals = new Map([
    ['first.json', '[]\n'],
    ['second.json', '[{"original":true}]\n'],
  ]);

  try {
    for (const [fileName, content] of originals) {
      await writeFile(join(databasePath, fileName), content, 'utf8');
    }

    let replacementCount = 0;
    await assert.rejects(
      commitSaveTransaction({
        databasePath,
        currentContents: originals,
        outputs: [
          { fileName: 'first.json', content: '[{"changed":1}]\n' },
          { fileName: 'second.json', content: '[{"changed":2}]\n' },
        ],
        replaceFile: async (source, destination) => {
          replacementCount += 1;
          if (replacementCount === 2) {
            throw new Error('simulated replacement failure');
          }
          await rename(source, destination);
        },
      }),
      /simulated replacement failure/,
    );

    for (const [fileName, content] of originals) {
      assert.equal(
        await readFile(join(databasePath, fileName), 'utf8'),
        content,
      );
    }
  } finally {
    await rm(databasePath, { recursive: true, force: true });
  }
});

interface FixtureContext {
  baseUrl: string;
  databasePath: string;
  snapshot: DatabaseSnapshot;
}

async function withFixture(
  run: (context: FixtureContext) => Promise<void>,
): Promise<void> {
  const databasePath = await mkdtemp(join(tmpdir(), 'ceditor2-database-api-'));
  const snapshot = createSnapshot();
  let server: Server | undefined;

  try {
    for (const { id, fileName } of ASSET_REGISTRY) {
      await writeFile(
        join(databasePath, fileName),
        `${JSON.stringify(snapshot[id], null, 2)}\n`,
        'utf8',
      );
    }
    await writeFile(join(databasePath, 'tiles.json'), LEGACY_TILES, 'utf8');

    server = await listen(databasePath);
    const address = server.address() as AddressInfo;
    await run({
      baseUrl: `http://127.0.0.1:${address.port}`,
      databasePath,
      snapshot,
    });
  } finally {
    if (server) {
      await close(server);
    }
    await rm(databasePath, { recursive: true, force: true });
  }
}

function createSnapshot(): DatabaseSnapshot {
  const snapshot = {} as DatabaseSnapshot;
  for (const { id } of ASSET_REGISTRY) {
    snapshot[id] = [];
  }
  return snapshot;
}

async function listen(databasePath: string): Promise<Server> {
  const server = createApp({ databasePath }).listen(0, '127.0.0.1');
  await new Promise<void>((resolve, reject) => {
    server.once('listening', resolve);
    server.once('error', reject);
  });
  return server;
}

async function close(server: Server): Promise<void> {
  await new Promise<void>((resolve, reject) => {
    server.close((error) => {
      if (error) reject(error);
      else resolve();
    });
  });
}

async function loadDatabase(baseUrl: string): Promise<DatabaseEnvelope> {
  const response = await fetch(`${baseUrl}/api/database`);
  assert.equal(response.status, 200);
  return (await response.json()) as DatabaseEnvelope;
}

async function putDatabase(baseUrl: string, body: unknown): Promise<Response> {
  return fetch(`${baseUrl}/api/database`, {
    method: 'PUT',
    headers: { 'content-type': 'application/json' },
    body: JSON.stringify(body),
  });
}

async function readManagedContents(
  databasePath: string,
): Promise<Map<string, string>> {
  return new Map(
    await Promise.all(
      ASSET_REGISTRY.map(
        async ({ fileName }) =>
          [
            fileName,
            await readFile(join(databasePath, fileName), 'utf8'),
          ] as const,
      ),
    ),
  );
}

async function statManagedFiles(
  databasePath: string,
): Promise<Map<string, { inode: number; modifiedMs: number }>> {
  return new Map(
    await Promise.all(
      ASSET_REGISTRY.map(async ({ fileName }) => {
        const info = await stat(join(databasePath, fileName));
        return [
          fileName,
          { inode: info.ino, modifiedMs: info.mtimeMs },
        ] as const;
      }),
    ),
  );
}
