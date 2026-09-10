import assert from 'node:assert/strict';
import test from 'node:test';

import {
  DatabaseClient,
  DatabaseConflictError,
  DatabaseProtocolError,
  DatabaseRequestError,
} from '../client/database/DatabaseClient';
import type {
  DatabaseSnapshot,
  SaveDatabaseRequest,
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

function jsonResponse(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { 'content-type': 'application/json' },
  });
}

function clientReturning(response: Response): DatabaseClient {
  const fetch = (() => Promise.resolve(response)) as typeof globalThis.fetch;
  return new DatabaseClient({ fetch });
}

test('GET parses an exact nine-collection database envelope', async () => {
  const assets = emptySnapshot();
  assets.itemTemplates.push({ apiName: 'potion', unknown: { retained: true } });
  let requestUrl: string | URL | Request | undefined;
  let requestInit: RequestInit | undefined;
  const fetch = ((url: string | URL | Request, init?: RequestInit) => {
    requestUrl = url;
    requestInit = init;
    return Promise.resolve(jsonResponse({ revision: 'revision-1', assets }));
  }) as typeof globalThis.fetch;

  const envelope = await new DatabaseClient({
    baseUrl: 'http://127.0.0.1:4100/',
    fetch,
  }).loadDatabase();

  assert.equal(requestUrl, 'http://127.0.0.1:4100/api/database');
  assert.equal(requestInit?.method, 'GET');
  assert.deepEqual(envelope, { revision: 'revision-1', assets });
});

const incompleteAssets: Record<string, unknown> = emptySnapshot();
delete incompleteAssets.maps;

for (const { label, assets } of [
  { label: 'a missing collection', assets: incompleteAssets },
  {
    label: 'an extra collection',
    assets: { ...emptySnapshot(), featTemplates: [] },
  },
  {
    label: 'a non-array collection',
    assets: { ...emptySnapshot(), maps: {} },
  },
]) {
  test(`GET rejects ${label}`, async () => {
    const client = clientReturning(
      jsonResponse({ revision: 'revision-1', assets }),
    );

    await assert.rejects(
      client.loadDatabase(),
      (error: unknown) => error instanceof DatabaseProtocolError,
    );
  });
}

test('GET rejects malformed JSON in a successful response', async () => {
  const client = clientReturning(
    new Response('{not-json', {
      status: 200,
      headers: { 'content-type': 'application/json' },
    }),
  );

  await assert.rejects(
    client.loadDatabase(),
    (error: unknown) => error instanceof DatabaseProtocolError,
  );
});

test('GET rejects an empty successful response', async () => {
  const client = clientReturning(new Response(null, { status: 200 }));

  await assert.rejects(
    client.loadDatabase(),
    (error: unknown) => error instanceof DatabaseProtocolError,
  );
});

test('network failures become protocol errors', async () => {
  const fetch = (() =>
    Promise.reject(
      new TypeError('connection refused'),
    )) as typeof globalThis.fetch;
  const client = new DatabaseClient({ fetch });

  await assert.rejects(
    client.loadDatabase(),
    (error: unknown) =>
      error instanceof DatabaseProtocolError &&
      error.message === 'Could not reach the database server',
  );
});

test('non-conflict API errors retain the server status, message, and body', async () => {
  const responseBody = { error: 'disk write failed', code: 'EIO' };
  const client = clientReturning(jsonResponse(responseBody, 500));

  await assert.rejects(client.loadDatabase(), (error: unknown) => {
    assert.ok(error instanceof DatabaseRequestError);
    assert.equal(error instanceof DatabaseConflictError, false);
    assert.equal(error.status, 500);
    assert.equal(error.message, 'disk write failed');
    assert.deepEqual(error.responseBody, responseBody);
    return true;
  });
});

test('HTTP 409 becomes a DatabaseConflictError', async () => {
  const responseBody = {
    error: 'revision conflict',
    expectedRevision: 'revision-2',
  };
  const client = clientReturning(jsonResponse(responseBody, 409));

  await assert.rejects(client.loadDatabase(), (error: unknown) => {
    assert.ok(error instanceof DatabaseConflictError);
    assert.equal(error.status, 409);
    assert.equal(error.message, 'revision conflict');
    assert.deepEqual(error.responseBody, responseBody);
    return true;
  });
});

test('PUT sends the full snapshot and parses a valid save response', async () => {
  let requestUrl: string | URL | Request | undefined;
  let requestInit: RequestInit | undefined;
  const fetch = ((url: string | URL | Request, init?: RequestInit) => {
    requestUrl = url;
    requestInit = init;
    return Promise.resolve(
      jsonResponse({
        revision: 'revision-2',
        changedFiles: ['items.json'],
      }),
    );
  }) as typeof globalThis.fetch;
  const client = new DatabaseClient({ baseUrl: '/editor', fetch });
  const request: SaveDatabaseRequest = {
    baseRevision: 'revision-1',
    assets: emptySnapshot(),
  };
  request.assets.itemTemplates.push({ apiName: 'potion' });

  const response = await client.saveDatabase(request);

  assert.equal(requestUrl, '/editor/api/database');
  assert.equal(requestInit?.method, 'PUT');
  assert.deepEqual(requestInit?.headers, {
    'Content-Type': 'application/json',
  });
  assert.deepEqual(JSON.parse(String(requestInit?.body)), request);
  assert.deepEqual(response, {
    revision: 'revision-2',
    changedFiles: ['items.json'],
  });
});

test('PUT rejects save responses containing unmanaged changed files', async () => {
  const client = clientReturning(
    jsonResponse({
      revision: 'revision-2',
      changedFiles: ['tiles.json'],
    }),
  );

  await assert.rejects(
    client.saveDatabase({
      baseRevision: 'revision-1',
      assets: emptySnapshot(),
    }),
    (error: unknown) => error instanceof DatabaseProtocolError,
  );
});
