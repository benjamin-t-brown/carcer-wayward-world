import assert from 'node:assert/strict';
import test from 'node:test';

import {
  ASSET_REGISTRY,
  DatabaseSession,
  type DatabaseSnapshot,
  type DatabaseTransport,
  type SaveDatabaseRequest,
} from '../../src/core/database/index.js';
import {
  createDefaultItemRecord,
  parseItemCollection,
} from '../../src/core/domain/items/index.js';

test('item replacement participates in an entire-database Save All snapshot', async () => {
  const assets = Object.fromEntries(
    ASSET_REGISTRY.map(({ id }) => [id, []]),
  ) as unknown as DatabaseSnapshot;
  assets.items = [
    { ...createDefaultItemRecord('Potion'), extension: { retained: true } },
  ];
  assets.abilities = [{ name: 'ABILITY_UNTOUCHED', unknown: 1 }];
  let request: SaveDatabaseRequest | undefined;
  const transport: DatabaseTransport = {
    async loadDatabase() {
      return { revision: 'revision-1', assets };
    },
    async saveDatabase(nextRequest) {
      request = nextRequest;
      return { revision: 'revision-2', changedFiles: ['items.json'] };
    },
  };
  const session = await DatabaseSession.load(transport);
  const items = parseItemCollection(session.collection('items'));
  items[0] = { ...items[0]!, label: 'Changed' };
  session.replaceCollection('items', items);
  await session.saveAll();

  assert.equal(request?.baseRevision, 'revision-1');
  assert.deepEqual(request?.assets.abilities, assets.abilities);
  assert.deepEqual(request?.assets.items, [
    {
      ...createDefaultItemRecord('Potion'),
      label: 'Changed',
      extension: { retained: true },
    },
  ]);
});
