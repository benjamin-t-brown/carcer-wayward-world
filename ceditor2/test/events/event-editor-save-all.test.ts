import assert from 'node:assert/strict';
import test from 'node:test';

import {
  ASSET_IDS,
  DatabaseSession,
  type DatabaseSnapshot,
  type DatabaseTransport,
  type JsonObject,
  type SaveDatabaseRequest,
} from '../../src/core/database/index.js';
import { EventDocument } from '../../src/core/domain/events/index.js';
import { createSpecialEvent } from '../../src/apps/events/editorModel.js';

test('event document edits participate in an atomic complete-database Save All', async () => {
  const assets = Object.fromEntries(
    ASSET_IDS.map((id) => [id, []]),
  ) as unknown as DatabaseSnapshot;
  assets.items.push({ name: 'untouched_item' });
  let saved: SaveDatabaseRequest | undefined;
  const transport: DatabaseTransport = {
    async loadDatabase() {
      return { revision: 'before', assets };
    },
    async saveDatabase(request) {
      saved = request;
      return { revision: 'after', changedFiles: ['special-events.json'] };
    },
  };
  const session = await DatabaseSession.load(transport);
  const document = EventDocument.from(createSpecialEvent('arrival'));
  document.updateHeader({ title: 'Arrival' });
  document.patchNode(0, { p: 'Welcome.', futureField: ['retained'] });
  session.replaceCollection('specialEvents', [document.snapshot()]);

  await session.saveAll();

  assert.deepEqual(Object.keys(saved!.assets), [...ASSET_IDS]);
  const event = saved!.assets.specialEvents[0]! as JsonObject;
  assert.equal(event.title, 'Arrival');
  assert.deepEqual(
    (event.children as { futureField: string[] }[])[0]?.futureField,
    ['retained'],
  );
  assert.deepEqual(saved!.assets.items, [{ name: 'untouched_item' }]);
  assert.equal(session.isDirty, false);
});
