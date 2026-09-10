import assert from 'node:assert/strict';
import {
  mkdtemp,
  readFile,
  readdir,
  rename,
  rm,
  stat,
  writeFile,
} from 'node:fs/promises';
import type { Server } from 'node:http';
import type { AddressInfo } from 'node:net';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import test from 'node:test';

import { createApp } from '../server/app';
import { commitSaveTransaction } from '../server/saveTransaction';
import { ASSET_TYPES } from '../shared/assetRegistry';
import type {
  DatabaseEnvelope,
  DatabaseSnapshot,
  SaveDatabaseResponse,
} from '../shared/databaseContract';

const UNMANAGED_TILES = '[{"legacy":true}]\n';
const SCRATCH_SENTINEL = 'keep this file exactly as-is\n';

test('GET /api/database loads exactly nine managed arrays with a stable revision', async () => {
  await withFixture(async ({ baseUrl, snapshot }) => {
    const first = await loadDatabase(baseUrl);
    const second = await loadDatabase(baseUrl);

    assert.equal(Object.keys(first.assets).length, 9);
    assert.deepEqual(first.assets, snapshot);
    assert.match(first.revision, /^[a-f0-9]{64}$/);
    assert.equal(second.revision, first.revision);
    assert.deepEqual(second.assets, first.assets);
  });
});

test('PUT /api/database changes only the semantically changed collection', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const loaded = await loadDatabase(baseUrl);
    const before = await readFixtureState(databasePath);
    const next = structuredClone(loaded.assets);
    next.itemTemplates.push({ name: 'NEW_TEST_ITEM' });

    const response = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: next,
    });
    assert.equal(response.status, 200);
    const result = (await response.json()) as SaveDatabaseResponse;
    assert.deepEqual(result.changedFiles, ['items.json']);
    assert.match(result.revision, /^[a-f0-9]{64}$/);
    assert.notEqual(result.revision, loaded.revision);

    assert.equal(
      await readFile(join(databasePath, 'items.json'), 'utf8'),
      `${JSON.stringify(next.itemTemplates, null, 2)}\n`,
    );

    const after = await readFixtureState(databasePath);
    for (const { file } of ASSET_TYPES) {
      if (file !== 'items.json') {
        assert.equal(after.get(file)?.content, before.get(file)?.content);
        assert.deepEqual(after.get(file)?.stat, before.get(file)?.stat);
      }
    }
    assertUnmanagedFilesUnchanged(after, before);
  });
});

test('PUT /api/database no-op preserves managed bytes, inode, mtime, and unmanaged files', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const loaded = await loadDatabase(baseUrl);
    const before = await readFixtureState(databasePath);

    const response = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: loaded.assets,
    });
    assert.equal(response.status, 200);
    const result = (await response.json()) as SaveDatabaseResponse;
    assert.deepEqual(result.changedFiles, []);
    assert.equal(result.revision, loaded.revision);
    assert.deepEqual(await readFixtureState(databasePath), before);
  });
});

test('stale revision rejects without writes after the external change', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const loaded = await loadDatabase(baseUrl);
    await writeFile(
      join(databasePath, 'abilities.json'),
      '[{"externallyChanged":true}]\n',
      'utf8',
    );
    const beforeAttempt = await readFixtureState(databasePath);
    const next = structuredClone(loaded.assets);
    next.itemTemplates.push({ name: 'MUST_NOT_BE_WRITTEN' });

    const response = await putDatabase(baseUrl, {
      baseRevision: loaded.revision,
      assets: next,
    });
    assert.equal(response.status, 409);
    assert.deepEqual(await readFixtureState(databasePath), beforeAttempt);
  });
});

test('concurrent saves serialize so the second writer sees a conflict', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const loaded = await loadDatabase(baseUrl);
    const first = structuredClone(loaded.assets);
    const second = structuredClone(loaded.assets);
    first.itemTemplates.push({ name: 'FIRST_CONCURRENT_EDIT' });
    second.itemTemplates.push({ name: 'SECOND_CONCURRENT_EDIT' });

    const responses = await Promise.all([
      putDatabase(baseUrl, {
        baseRevision: loaded.revision,
        assets: first,
      }),
      putDatabase(baseUrl, {
        baseRevision: loaded.revision,
        assets: second,
      }),
    ]);

    assert.deepEqual(responses.map(({ status }) => status).sort(), [200, 409]);

    const saved = JSON.parse(
      await readFile(join(databasePath, 'items.json'), 'utf8'),
    ) as Array<{ name?: string }>;
    const savedNames = saved.map(({ name }) => name);
    assert.equal(
      savedNames.includes('FIRST_CONCURRENT_EDIT'),
      !savedNames.includes('SECOND_CONCURRENT_EDIT'),
    );
  });
});

test('invalid JSON request bodies return 400 and make zero writes', async () => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const before = await readFixtureState(databasePath);
    const response = await fetch(`${baseUrl}/api/database`, {
      method: 'PUT',
      headers: { 'content-type': 'application/json' },
      body: '{"baseRevision":',
    });

    assert.equal(response.status, 400);
    assert.deepEqual(await readFixtureState(databasePath), before);
  });
});

test('incomplete, non-array, and extra collection requests make zero writes', async (t) => {
  await withFixture(async ({ baseUrl, databasePath }) => {
    const loaded = await loadDatabase(baseUrl);

    await t.test('incomplete snapshot', async () => {
      const assets = structuredClone(loaded.assets) as Record<string, unknown>;
      delete assets.maps;
      await assertRejectedWithoutWrites(databasePath, baseUrl, {
        baseRevision: loaded.revision,
        assets,
      });
    });

    await t.test('non-array collection', async () => {
      const assets = structuredClone(loaded.assets) as Record<string, unknown>;
      assets.maps = {};
      await assertRejectedWithoutWrites(databasePath, baseUrl, {
        baseRevision: loaded.revision,
        assets,
      });
    });

    await t.test('extra collection', async () => {
      const assets = structuredClone(loaded.assets) as Record<string, unknown>;
      assets.featTemplates = [];
      await assertRejectedWithoutWrites(databasePath, baseUrl, {
        baseRevision: loaded.revision,
        assets,
      });
    });
  });
});

test('failed multi-file replacement rolls back replaced files and cleans staging', async () => {
  const databasePath = await mkdtemp(join(tmpdir(), 'ceditor-rollback-'));
  const originals = new Map([
    ['first.json', '[]\n'],
    ['second.json', '[{"original":true}]\n'],
  ]);

  try {
    for (const [fileName, content] of originals) {
      await writeFile(join(databasePath, fileName), content, 'utf8');
    }
    await writeFile(
      join(databasePath, 'scratch-sentinel.txt'),
      SCRATCH_SENTINEL,
      'utf8',
    );

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
    assert.equal(
      await readFile(join(databasePath, 'scratch-sentinel.txt'), 'utf8'),
      SCRATCH_SENTINEL,
    );
    assert.deepEqual((await readdir(databasePath)).sort(), [
      'first.json',
      'scratch-sentinel.txt',
      'second.json',
    ]);
  } finally {
    await rm(databasePath, { recursive: true, force: true });
  }
});

interface FixtureContext {
  baseUrl: string;
  databasePath: string;
  snapshot: DatabaseSnapshot;
}

interface FileState {
  content: string;
  stat: { inode: number; modifiedMs: number };
}

async function withFixture(
  run: (context: FixtureContext) => Promise<void>,
): Promise<void> {
  const databasePath = await mkdtemp(join(tmpdir(), 'ceditor-database-api-'));
  const snapshot = {} as DatabaseSnapshot;
  let server: Server | undefined;

  try {
    for (const [index, { id, file }] of ASSET_TYPES.entries()) {
      const records = [{ fixture: id, ordinal: index }];
      snapshot[id] = records;
      const deliberatelyNonCanonical = ` [ ${JSON.stringify(records[0])} ] \n`;
      await writeFile(
        join(databasePath, file),
        deliberatelyNonCanonical,
        'utf8',
      );
    }
    await writeFile(join(databasePath, 'tiles.json'), UNMANAGED_TILES, 'utf8');
    await writeFile(
      join(databasePath, 'scratch-sentinel.txt'),
      SCRATCH_SENTINEL,
      'utf8',
    );

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
      if (error) {
        reject(error);
      } else {
        resolve();
      }
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

async function assertRejectedWithoutWrites(
  databasePath: string,
  baseUrl: string,
  body: unknown,
): Promise<void> {
  const before = await readFixtureState(databasePath);
  const response = await putDatabase(baseUrl, body);
  assert.equal(response.status, 400);
  assert.deepEqual(await readFixtureState(databasePath), before);
}

async function readFixtureState(
  databasePath: string,
): Promise<Map<string, FileState>> {
  const names = [
    ...ASSET_TYPES.map(({ file }) => file),
    'tiles.json',
    'scratch-sentinel.txt',
  ];
  return new Map(
    await Promise.all(
      names.map(async (fileName) => {
        const [content, info] = await Promise.all([
          readFile(join(databasePath, fileName), 'utf8'),
          stat(join(databasePath, fileName)),
        ]);
        return [
          fileName,
          {
            content,
            stat: { inode: info.ino, modifiedMs: info.mtimeMs },
          },
        ] as const;
      }),
    ),
  );
}

function assertUnmanagedFilesUnchanged(
  actual: ReadonlyMap<string, FileState>,
  expected: ReadonlyMap<string, FileState>,
): void {
  for (const fileName of ['tiles.json', 'scratch-sentinel.txt']) {
    assert.deepEqual(actual.get(fileName), expected.get(fileName));
  }
}
