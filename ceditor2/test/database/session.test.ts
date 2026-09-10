import assert from 'node:assert/strict';
import test from 'node:test';

import {
  ASSET_IDS,
  DatabaseSession,
  type DatabaseEnvelope,
  type DatabaseSnapshot,
  type DatabaseTransport,
  type JsonArray,
  type SaveDatabaseRequest,
  type SaveDatabaseResponse,
} from '../../src/core/database/index.js';

test('the default client keeps the browser fetch receiver', async () => {
  const originalFetch = globalThis.fetch;
  const receivers = new Set<unknown>();
  globalThis.fetch = function (this: unknown) {
    receivers.add(this);
    return Promise.resolve(
      new Response(
        JSON.stringify({ revision: 'revision-1', assets: emptySnapshot() }),
        { headers: { 'content-type': 'application/json' } },
      ),
    );
  };
  try {
    const { DatabaseClient } =
      await import('../../src/core/database/DatabaseClient.js');
    await new DatabaseClient().loadDatabase();
    assert.equal(receivers.has(globalThis), true);
  } finally {
    globalThis.fetch = originalFetch;
  }
});

function emptySnapshot(): DatabaseSnapshot {
  return {
    statusEffects: [],
    abilities: [],
    items: [],
    spells: [],
    characters: [],
    maps: [],
    mapGrids: [],
    tilesets: [],
    specialEvents: [],
  };
}

class FakeClient implements DatabaseTransport {
  readonly saveRequests: SaveDatabaseRequest[] = [];
  response: SaveDatabaseResponse = {
    revision: 'revision-2',
    changedFiles: ['items.json'],
  };

  constructor(readonly envelope: DatabaseEnvelope) {}

  async loadDatabase(): Promise<DatabaseEnvelope> {
    return this.envelope;
  }

  async saveDatabase(
    request: SaveDatabaseRequest,
  ): Promise<SaveDatabaseResponse> {
    this.saveRequests.push(structuredClone(request));
    return this.response;
  }
}

test('loads an isolated full snapshot', async () => {
  const assets = emptySnapshot();
  assets.items.push({ name: 'Potion', unknownField: { retained: true } });
  const client = new FakeClient({ revision: 'revision-1', assets });

  const session = await DatabaseSession.load(client);
  assets.items[0] = { name: 'Changed outside session' };

  assert.equal(session.baseRevision, 'revision-1');
  assert.deepEqual(session.collection('items'), [
    { name: 'Potion', unknownField: { retained: true } },
  ]);
  assert.equal(session.isDirty, false);
});

test('returned snapshots and collections cannot mutate session state', async () => {
  const assets = emptySnapshot();
  assets.items.push({ name: 'Potion' });
  const session = await DatabaseSession.load(
    new FakeClient({ revision: 'revision-1', assets }),
  );

  session.collection('items').push({ name: 'Untracked' });
  session.snapshot().items[0] = { name: 'Also untracked' };

  assert.deepEqual(session.collection('items'), [{ name: 'Potion' }]);
  assert.equal(session.isDirty, false);
});

test('replacement and mutation mark only their asset collections dirty', async () => {
  const session = await DatabaseSession.load(
    new FakeClient({ revision: 'revision-1', assets: emptySnapshot() }),
  );
  const replacement: JsonArray = [{ name: 'Sword', extension: 7 }];

  session.replaceCollection('items', replacement);
  replacement[0] = { name: 'Changed outside session' };
  session.mutateCollection('characters', (characters) => {
    characters.push({ name: 'Guard', editorOnly: 'preserved' });
  });

  assert.deepEqual(session.collection('items'), [
    { name: 'Sword', extension: 7 },
  ]);
  assert.deepEqual(session.collection('characters'), [
    { name: 'Guard', editorOnly: 'preserved' },
  ]);
  assert.deepEqual([...session.dirtyAssetIds].sort(), ['characters', 'items']);
});

test('failed mutations do not change the session or mark it dirty', async () => {
  const assets = emptySnapshot();
  assets.items.push({ name: 'Potion' });
  const session = await DatabaseSession.load(
    new FakeClient({ revision: 'revision-1', assets }),
  );

  assert.throws(() => {
    session.mutateCollection('items', (items) => {
      items.push({ name: 'Sword' });
      throw new Error('cancel mutation');
    });
  }, /cancel mutation/);

  assert.deepEqual(session.collection('items'), [{ name: 'Potion' }]);
  assert.equal(session.isDirty, false);
});

test('saveAll sends every collection and advances the baseline revision', async () => {
  const client = new FakeClient({
    revision: 'revision-1',
    assets: emptySnapshot(),
  });
  const session = await DatabaseSession.load(client);
  session.mutateCollection('items', (items) => {
    items.push({ name: 'Potion', unknown: ['kept'] });
  });

  const response = await session.saveAll();

  assert.deepEqual(response, client.response);
  assert.equal(client.saveRequests.length, 1);
  assert.equal(client.saveRequests[0]?.baseRevision, 'revision-1');
  assert.deepEqual(
    Object.keys(client.saveRequests[0]?.assets ?? {}).sort(),
    [...ASSET_IDS].sort(),
  );
  assert.deepEqual(client.saveRequests[0]?.assets.items, [
    { name: 'Potion', unknown: ['kept'] },
  ]);
  assert.equal(session.baseRevision, 'revision-2');
  assert.equal(session.isDirty, false);
});

test('a failed save retains its revision and dirty state', async () => {
  const client = new FakeClient({
    revision: 'revision-1',
    assets: emptySnapshot(),
  });
  client.saveDatabase = async () => {
    throw new Error('save rejected');
  };
  const session = await DatabaseSession.load(client);
  session.replaceCollection('spells', [{ name: 'Spark' }]);

  await assert.rejects(session.saveAll(), /save rejected/);

  assert.equal(session.baseRevision, 'revision-1');
  assert.deepEqual([...session.dirtyAssetIds], ['spells']);
});

test('edits made while saving remain dirty', async () => {
  let finishSave: ((response: SaveDatabaseResponse) => void) | undefined;
  const client = new FakeClient({
    revision: 'revision-1',
    assets: emptySnapshot(),
  });
  client.saveDatabase = async (request) => {
    client.saveRequests.push(structuredClone(request));
    return new Promise<SaveDatabaseResponse>((resolve) => {
      finishSave = resolve;
    });
  };
  const session = await DatabaseSession.load(client);
  session.replaceCollection('items', [{ name: 'Potion' }]);

  const save = session.saveAll();
  session.mutateCollection('items', (items) => {
    items.push({ name: 'Sword' });
  });
  finishSave?.({ revision: 'revision-2', changedFiles: ['items.json'] });
  await save;

  assert.equal(session.baseRevision, 'revision-2');
  assert.deepEqual([...session.dirtyAssetIds], ['items']);
  assert.deepEqual(client.saveRequests[0]?.assets.items, [{ name: 'Potion' }]);
  assert.deepEqual(session.collection('items'), [
    { name: 'Potion' },
    { name: 'Sword' },
  ]);
});
