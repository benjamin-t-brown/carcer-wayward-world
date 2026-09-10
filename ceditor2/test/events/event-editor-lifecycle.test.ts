import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import type { JsonArray, JsonObject } from '../../src/core/database/index.js';
import {
  deleteEventAcrossDatabase,
  previewEventReferences,
  renameEventAcrossDatabase,
  type EventDatabaseRecords,
} from '../../src/apps/events/eventLifecycle.js';

function fixture(): EventDatabaseRecords {
  return {
    specialEvents: [
      {
        id: 'target',
        title: 'Target',
        eventType: 'TALK',
        icon: '',
        vars: [],
        children: [],
        retained: { event: true },
      },
      {
        id: 'importer',
        title: 'Importer',
        eventType: 'MODAL',
        icon: '',
        vars: [
          { id: 'keep', key: 'A', value: 'one', importFrom: '' },
          { id: 'drop', key: '', value: '', importFrom: 'target', custom: 4 },
        ],
        children: [],
      },
    ],
    maps: [
      {
        name: 'map_one',
        eventTriggers: [
          { eventId: 'target', l: 0, i: 4, requiresLook: true },
          { eventId: 'other', l: 0, i: 5 },
        ],
      },
    ],
    characters: [
      {
        name: 'npc',
        talk: { talkName: 'target', portraitName: 'portrait', custom: true },
      },
    ],
    items: [{ name: 'scroll', useSpecialEvent: 'target', retained: 9 }],
  };
}

function at(values: JsonArray, index: number): JsonObject {
  return values[index] as JsonObject;
}

test('event rename previews and atomically updates every structured reference', () => {
  const source = fixture();
  const before = JSON.stringify(source);
  const preview = previewEventReferences(source, 'target');
  assert.deepEqual(preview.references.map(({ kind }) => kind).sort(), [
    'character-talk',
    'event-import',
    'item-use',
    'map-trigger',
  ]);

  const result = renameEventAcrossDatabase(source, 'target', 'renamed');
  assert.equal(at(result.specialEvents, 0).id, 'renamed');
  assert.equal(
    ((at(result.maps, 0).eventTriggers as JsonObject[])[0] as JsonObject)
      .eventId,
    'renamed',
  );
  assert.equal(
    (at(result.characters, 0).talk as JsonObject).talkName,
    'renamed',
  );
  assert.equal(at(result.items, 0).useSpecialEvent, 'renamed');
  assert.equal(
    ((at(result.specialEvents, 1).vars as JsonObject[])[1] as JsonObject)
      .importFrom,
    'renamed',
  );
  assert.equal(
    JSON.stringify(source),
    before,
    'the input database is immutable',
  );
  assert.deepEqual(at(result.specialEvents, 0).retained, { event: true });
});

test('selected rename references leave unchecked references untouched and reject collisions', () => {
  const source = fixture();
  const preview = previewEventReferences(source, 'target');
  const importPath = preview.references.find(
    ({ kind }) => kind === 'event-import',
  )!.path;
  const result = renameEventAcrossDatabase(
    source,
    'target',
    'renamed',
    new Set([importPath]),
  );
  assert.equal(
    ((at(result.specialEvents, 1).vars as JsonObject[])[1] as JsonObject)
      .importFrom,
    'renamed',
  );
  assert.equal(at(result.items, 0).useSpecialEvent, 'target');
  assert.throws(
    () => renameEventAcrossDatabase(source, 'target', 'importer'),
    /already exists/,
  );
});

test('event deletion explicitly removes selected dependent records and clears optional links', () => {
  const result = deleteEventAcrossDatabase(fixture(), 'target');
  assert.deepEqual(
    result.specialEvents.map((record) => (record as JsonObject).id),
    ['importer'],
  );
  assert.deepEqual(at(result.maps, 0).eventTriggers, [
    { eventId: 'other', l: 0, i: 5 },
  ]);
  assert.deepEqual(at(result.characters, 0).talk, {
    talkName: '',
    portraitName: 'portrait',
    custom: true,
  });
  assert.equal(Object.hasOwn(at(result.items, 0), 'useSpecialEvent'), false);
  assert.deepEqual(at(result.items, 0).retained, 9);
  assert.deepEqual(at(result.specialEvents, 0).vars, [
    { id: 'keep', key: 'A', value: 'one', importFrom: '' },
  ]);
});

test('the real database event reference preview finds maps, characters, and imports', async () => {
  const read = async (name: string): Promise<JsonArray> =>
    JSON.parse(
      await readFile(
        new URL(`../../../src/assets/db/${name}.json`, import.meta.url),
        'utf8',
      ),
    ) as JsonArray;
  const database: EventDatabaseRecords = {
    specialEvents: await read('special-events'),
    maps: await read('maps'),
    characters: await read('characters'),
    items: await read('items'),
  };
  assert.equal(
    previewEventReferences(database, 'alinea_sign_warehouse1').references
      .length,
    2,
  );
  assert.equal(
    previewEventReferences(database, 'alinea_claire').references.some(
      ({ kind }) => kind === 'character-talk',
    ),
    true,
  );
  assert.equal(
    previewEventReferences(database, 'util_aspect').references.filter(
      ({ kind }) => kind === 'event-import',
    ).length,
    12,
  );
});
