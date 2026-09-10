import assert from 'node:assert/strict';
import test from 'node:test';

import {
  ASSET_IDS,
  DatabaseSession,
  type DatabaseSnapshot,
  type DatabaseTransport,
  type SaveDatabaseRequest,
} from '../../src/core/database/index.js';
import { createDefaultCharacterRecord } from '../../src/core/domain/characters/index.js';

test('character edits participate in an atomic complete-database Save All', async () => {
  const assets = Object.fromEntries(
    ASSET_IDS.map((id) => [id, []]),
  ) as unknown as DatabaseSnapshot;
  let saved: SaveDatabaseRequest | undefined;
  const transport: DatabaseTransport = {
    async loadDatabase() {
      return { revision: 'before', assets };
    },
    async saveDatabase(request) {
      saved = request;
      return { revision: 'after', changedFiles: ['characters.json'] };
    },
  };
  const session = await DatabaseSession.load(transport);
  session.replaceCollection('characters', [
    createDefaultCharacterRecord('npc'),
  ]);

  await session.saveAll();

  assert.deepEqual(Object.keys(saved!.assets), [...ASSET_IDS]);
  assert.deepEqual(saved!.assets.characters, [
    createDefaultCharacterRecord('npc'),
  ]);
  assert.deepEqual(saved!.assets.maps, []);
  assert.equal(session.isDirty, false);
});
