import assert from 'node:assert/strict';
import test from 'node:test';

import { DatabaseSession } from '../client/database/DatabaseSession';
import { ASSET_TYPES } from '../shared/assetRegistry';
import type {
  DatabaseEnvelope,
  DatabaseSnapshot,
  DatabaseTransport,
  JsonArray,
  SaveDatabaseRequest,
  SaveDatabaseResponse,
} from '../shared/databaseContract';

function emptySnapshot(): DatabaseSnapshot {
  return {
    itemTemplates: [],
    abilityTemplates: [],
    spellTemplates: [],
    statusEffectTemplates: [],
    characterTemplates: [],
    specialEvents: [],
    tilesetTemplates: [],
    maps: [],
    mapGrids: [],
  };
}

function deferred<T>(): {
  promise: Promise<T>;
  resolve: (value: T) => void;
  reject: (reason?: unknown) => void;
} {
  let resolve!: (value: T) => void;
  let reject!: (reason?: unknown) => void;
  const promise = new Promise<T>((resolvePromise, rejectPromise) => {
    resolve = resolvePromise;
    reject = rejectPromise;
  });
  return { promise, resolve, reject };
}

class FakeTransport implements DatabaseTransport {
  readonly saveRequests: SaveDatabaseRequest[] = [];
  saveResponse: Promise<SaveDatabaseResponse> = Promise.resolve({
    revision: 'revision-2',
    changedFiles: [],
  });

  constructor(readonly envelope: DatabaseEnvelope) {}

  loadDatabase(): Promise<DatabaseEnvelope> {
    return Promise.resolve(this.envelope);
  }

  saveDatabase(request: SaveDatabaseRequest): Promise<SaveDatabaseResponse> {
    this.saveRequests.push(structuredClone(request));
    return this.saveResponse;
  }
}

test('loads and exposes defensive copies without creating dirty state', async () => {
  const assets = emptySnapshot();
  assets.itemTemplates.push({
    apiName: 'potion',
    extension: { retained: true },
  });
  const transport = new FakeTransport({
    revision: 'revision-1',
    assets,
  });

  const session = await DatabaseSession.load(transport);
  assets.itemTemplates[0] = { apiName: 'changed-outside-session' };
  session.collection('itemTemplates').push({ apiName: 'untracked' });
  session.snapshot().itemTemplates[0] = { apiName: 'also-untracked' };

  assert.equal(session.baseRevision, 'revision-1');
  assert.deepEqual(session.collection('itemTemplates'), [
    { apiName: 'potion', extension: { retained: true } },
  ]);
  assert.equal(session.isDirty, false);
  assert.deepEqual([...session.dirtyAssetIds], []);
});

test('collection changes are cloned and identify only their dirty asset IDs', async () => {
  const session = await DatabaseSession.load(
    new FakeTransport({ revision: 'revision-1', assets: emptySnapshot() }),
  );
  const replacement: JsonArray = [
    { apiName: 'sword', extension: { power: 7 } },
  ];

  session.replaceCollection('itemTemplates', replacement);
  replacement[0] = { apiName: 'changed-outside-session', extension: {} };
  session.mutateCollection('characterTemplates', (characters) => {
    characters.push({ apiName: 'guard', editorOnly: 'preserved' });
  });

  assert.deepEqual(session.collection('itemTemplates'), [
    { apiName: 'sword', extension: { power: 7 } },
  ]);
  assert.deepEqual(session.collection('characterTemplates'), [
    { apiName: 'guard', editorOnly: 'preserved' },
  ]);
  assert.deepEqual([...session.dirtyAssetIds].sort(), [
    'characterTemplates',
    'itemTemplates',
  ]);
});

test('failed collection mutation leaves the snapshot and dirty set unchanged', async () => {
  const assets = emptySnapshot();
  assets.itemTemplates.push({ apiName: 'potion' });
  const session = await DatabaseSession.load(
    new FakeTransport({ revision: 'revision-1', assets }),
  );

  assert.throws(() => {
    session.mutateCollection('itemTemplates', (items) => {
      items.push({ apiName: 'sword' });
      throw new Error('cancel mutation');
    });
  }, /cancel mutation/);

  assert.deepEqual(session.collection('itemTemplates'), [
    { apiName: 'potion' },
  ]);
  assert.equal(session.isDirty, false);
});

test('saveAll sends all nine collections and advances the base revision', async () => {
  const transport = new FakeTransport({
    revision: 'revision-1',
    assets: emptySnapshot(),
  });
  transport.saveResponse = Promise.resolve({
    revision: 'revision-2',
    changedFiles: ['items.json'],
  });
  const session = await DatabaseSession.load(transport);
  session.replaceCollection('itemTemplates', [
    { apiName: 'potion', unknown: ['kept'] },
  ]);

  const response = await session.saveAll();

  assert.deepEqual(response, {
    revision: 'revision-2',
    changedFiles: ['items.json'],
  });
  assert.equal(transport.saveRequests.length, 1);
  assert.equal(transport.saveRequests[0]?.baseRevision, 'revision-1');
  assert.deepEqual(
    Object.keys(transport.saveRequests[0]?.assets ?? {}).sort(),
    ASSET_TYPES.map(({ id }) => id).sort(),
  );
  assert.deepEqual(transport.saveRequests[0]?.assets.itemTemplates, [
    { apiName: 'potion', unknown: ['kept'] },
  ]);
  assert.equal(session.baseRevision, 'revision-2');
  assert.equal(session.isDirty, false);
});

test('a save clears only captured versions while an in-flight edit stays dirty', async () => {
  const pendingSave = deferred<SaveDatabaseResponse>();
  const transport = new FakeTransport({
    revision: 'revision-1',
    assets: emptySnapshot(),
  });
  transport.saveResponse = pendingSave.promise;
  const session = await DatabaseSession.load(transport);
  session.replaceCollection('itemTemplates', [{ apiName: 'potion' }]);
  session.replaceCollection('spellTemplates', [{ apiName: 'spark' }]);

  const save = session.saveAll();
  session.mutateCollection('itemTemplates', (items) => {
    items.push({ apiName: 'sword' });
  });
  pendingSave.resolve({
    revision: 'revision-2',
    changedFiles: ['items.json', 'spells.json'],
  });
  await save;

  assert.equal(session.baseRevision, 'revision-2');
  assert.deepEqual([...session.dirtyAssetIds], ['itemTemplates']);
  assert.deepEqual(transport.saveRequests[0]?.assets.itemTemplates, [
    { apiName: 'potion' },
  ]);
  assert.deepEqual(session.collection('itemTemplates'), [
    { apiName: 'potion' },
    { apiName: 'sword' },
  ]);
  assert.deepEqual(session.collection('spellTemplates'), [
    { apiName: 'spark' },
  ]);
});

test('overlapping saves are queued and the later save uses the advanced revision', async () => {
  const firstResponse = deferred<SaveDatabaseResponse>();
  const requests: SaveDatabaseRequest[] = [];
  const transport: DatabaseTransport = {
    loadDatabase: async () => ({
      revision: 'revision-1',
      assets: emptySnapshot(),
    }),
    saveDatabase: async (request) => {
      requests.push(structuredClone(request));
      if (requests.length === 1) {
        return firstResponse.promise;
      }
      return { revision: 'revision-3', changedFiles: ['items.json'] };
    },
  };
  const session = await DatabaseSession.load(transport);
  session.replaceCollection('itemTemplates', [{ apiName: 'potion' }]);

  const firstSave = session.saveAll();
  session.replaceCollection('itemTemplates', [{ apiName: 'sword' }]);
  const secondSave = session.saveAll();

  assert.equal(requests.length, 1);
  firstResponse.resolve({
    revision: 'revision-2',
    changedFiles: ['items.json'],
  });
  await firstSave;
  await secondSave;

  assert.equal(requests.length, 2);
  assert.equal(requests[0]?.baseRevision, 'revision-1');
  assert.equal(requests[1]?.baseRevision, 'revision-2');
  assert.deepEqual(requests[0]?.assets.itemTemplates, [{ apiName: 'potion' }]);
  assert.deepEqual(requests[1]?.assets.itemTemplates, [{ apiName: 'sword' }]);
  assert.equal(session.baseRevision, 'revision-3');
  assert.equal(session.isDirty, false);
});

for (const rejection of [
  new Error('save failed'),
  Object.assign(new Error('revision conflict'), { status: 409 }),
]) {
  test(`${rejection.message} preserves revision, snapshot, and dirty state`, async () => {
    const transport = new FakeTransport({
      revision: 'revision-1',
      assets: emptySnapshot(),
    });
    transport.saveResponse = Promise.reject(rejection);
    const session = await DatabaseSession.load(transport);
    session.replaceCollection('mapGrids', [
      { apiName: 'overworld', maps: ['partition-a'] },
    ]);
    const beforeSave = session.snapshot();

    await assert.rejects(session.saveAll(), rejection);

    assert.equal(session.baseRevision, 'revision-1');
    assert.deepEqual(session.snapshot(), beforeSave);
    assert.deepEqual([...session.dirtyAssetIds], ['mapGrids']);
  });
}
