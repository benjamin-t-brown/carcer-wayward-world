import assert from 'node:assert/strict';
import test from 'node:test';

import { DatabaseSession } from '../client/database/DatabaseSession';
import type {
  CarcerMapTemplate,
  CharacterTemplate,
  GameEvent,
  ItemTemplate,
  MapGridTemplate,
} from '../client/types/assets';
import { prepareMapRenameCandidates } from '../client/utils/mapPersistence';
import { prepareAtomicGameEventRename } from '../client/utils/specialEventSavePreparation';
import type {
  DatabaseEnvelope,
  DatabaseSnapshot,
  DatabaseTransport,
  JsonArray,
  SaveDatabaseRequest,
  SaveDatabaseResponse,
} from '../shared/databaseContract';

class TransactionTransport implements DatabaseTransport {
  readonly requests: SaveDatabaseRequest[] = [];
  nextError: Error | undefined;
  nextRevision = 2;

  constructor(private readonly envelope: DatabaseEnvelope) {}

  async loadDatabase(): Promise<DatabaseEnvelope> {
    return structuredClone(this.envelope);
  }

  async saveDatabase(
    request: SaveDatabaseRequest,
  ): Promise<SaveDatabaseResponse> {
    this.requests.push(structuredClone(request));
    if (this.nextError) {
      const error = this.nextError;
      this.nextError = undefined;
      throw error;
    }
    return {
      revision: `revision-${this.nextRevision++}`,
      changedFiles: [],
    };
  }
}

test('map rename and every grid cell reference are sent in one database snapshot', async () => {
  const maps = [map('old-map'), map('standalone')];
  const mapGrids = [
    grid([
      ['old-map', 'standalone'],
      ['old-map', ''],
    ]),
  ];
  const snapshot = databaseSnapshot({ maps, mapGrids });
  const transport = new TransactionTransport({
    revision: 'revision-1',
    assets: snapshot,
  });
  const session = await DatabaseSession.load(transport);
  const candidate = prepareMapRenameCandidates(maps, mapGrids, 'old-map', {
    ...maps[0]!,
    name: 'new-map',
    label: ' New map ',
  });

  assert.ok(candidate);
  session.replaceCollection('maps', asJsonArray(candidate.maps));
  session.replaceCollection('mapGrids', asJsonArray(candidate.mapGrids));
  assert.deepEqual([...session.dirtyAssetIds].sort(), ['mapGrids', 'maps']);

  await session.saveAll();

  assert.equal(transport.requests.length, 1);
  const saved = transport.requests[0]!.assets;
  assert.deepEqual(
    (saved.maps as unknown as CarcerMapTemplate[]).map(({ name }) => name),
    ['new-map', 'standalone'],
  );
  assert.deepEqual((saved.mapGrids as unknown as MapGridTemplate[])[0]?.cells, [
    ['new-map', 'standalone'],
    ['new-map', ''],
  ]);
  assert.deepEqual(saved.itemTemplates, snapshot.itemTemplates);
  assert.equal(session.isDirty, false);
});

test('failed map-and-grid save retains one coherent draft for Save All retry', async () => {
  const maps = [map('old-map')];
  const mapGrids = [grid([['old-map']])];
  const transport = new TransactionTransport({
    revision: 'revision-1',
    assets: databaseSnapshot({ maps, mapGrids }),
  });
  const session = await DatabaseSession.load(transport);
  const candidate = prepareMapRenameCandidates(maps, mapGrids, 'old-map', {
    ...maps[0]!,
    name: 'new-map',
  });
  assert.ok(candidate);
  session.replaceCollection('maps', asJsonArray(candidate.maps));
  session.replaceCollection('mapGrids', asJsonArray(candidate.mapGrids));
  transport.nextError = new Error('temporary save failure');

  await assert.rejects(session.saveAll(), /temporary save failure/);

  assert.equal(session.baseRevision, 'revision-1');
  assert.deepEqual([...session.dirtyAssetIds].sort(), ['mapGrids', 'maps']);
  assertMapRenameIsCoherent(session.snapshot(), 'new-map');

  await session.saveAll();

  assert.equal(transport.requests.length, 2);
  assert.deepEqual(
    transport.requests[1]?.assets,
    transport.requests[0]?.assets,
  );
  assertMapRenameIsCoherent(transport.requests[1]!.assets, 'new-map');
  assert.equal(session.baseRevision, 'revision-2');
  assert.equal(session.isDirty, false);
});

test('special-event rename and references remain coherent through a failed save and retry', async () => {
  const gameEvents = [
    event('old-event', [{ id: 'self', importFrom: 'old-event' }]),
    event('consumer', [{ id: 'source', importFrom: 'old-event' }]),
    event('unrelated', [{ id: 'other', importFrom: 'another-event' }]),
  ];
  const maps = [
    {
      ...map('event-map'),
      eventTriggers: [
        { l: 0, i: 0, eventId: 'old-event' },
        { l: 0, i: 1, eventId: 'another-event' },
      ],
    },
  ];
  const characters = [
    { name: 'speaker', talk: { talkName: 'old-event', portraitName: '' } },
    { name: 'other', talk: { talkName: 'another-event', portraitName: '' } },
  ] as CharacterTemplate[];
  const items = [
    { name: 'letter', useSpecialEvent: 'old-event' },
    { name: 'other', useSpecialEvent: 'another-event' },
  ] as ItemTemplate[];
  const originals = structuredClone({ gameEvents, maps, characters, items });
  const prepared = prepareAtomicGameEventRename({
    gameEvents,
    maps,
    characters,
    items,
    previousEventId: 'old-event',
    updatedGameEvent: {
      ...gameEvents[0]!,
      id: 'new-event',
      title: ' New title ',
    },
  });
  const transport = new TransactionTransport({
    revision: 'revision-1',
    assets: databaseSnapshot({
      maps,
      specialEvents: gameEvents,
      characterTemplates: characters,
      itemTemplates: items,
    }),
  });
  const session = await DatabaseSession.load(transport);

  session.replaceCollection('specialEvents', asJsonArray(prepared.gameEvents));
  if (prepared.maps) {
    session.replaceCollection('maps', asJsonArray(prepared.maps));
  }
  if (prepared.characters) {
    session.replaceCollection(
      'characterTemplates',
      asJsonArray(prepared.characters),
    );
  }
  if (prepared.items) {
    session.replaceCollection('itemTemplates', asJsonArray(prepared.items));
  }

  assert.deepEqual({ gameEvents, maps, characters, items }, originals);
  assert.deepEqual([...session.dirtyAssetIds].sort(), [
    'characterTemplates',
    'itemTemplates',
    'maps',
    'specialEvents',
  ]);
  transport.nextError = new Error('temporary event save failure');

  await assert.rejects(session.saveAll(), /temporary event save failure/);

  assert.equal(session.baseRevision, 'revision-1');
  assertEventRenameIsCoherent(session.snapshot());
  assert.deepEqual([...session.dirtyAssetIds].sort(), [
    'characterTemplates',
    'itemTemplates',
    'maps',
    'specialEvents',
  ]);
  await session.saveAll();

  assert.equal(transport.requests.length, 2);
  assert.deepEqual(
    transport.requests[1]?.assets,
    transport.requests[0]?.assets,
  );
  assertEventRenameIsCoherent(transport.requests[1]!.assets);
  assert.equal(session.isDirty, false);
});

function databaseSnapshot(
  replacements: Partial<Record<keyof DatabaseSnapshot, unknown[]>> = {},
): DatabaseSnapshot {
  return {
    itemTemplates: [{ name: 'untouched-item' }],
    abilityTemplates: [],
    spellTemplates: [],
    statusEffectTemplates: [],
    characterTemplates: [],
    specialEvents: [],
    tilesetTemplates: [],
    maps: [],
    mapGrids: [],
    ...Object.fromEntries(
      Object.entries(replacements).map(([id, records]) => [
        id,
        asJsonArray(records ?? []),
      ]),
    ),
  };
}

function asJsonArray(value: unknown): JsonArray {
  return value as JsonArray;
}

function map(name: string): CarcerMapTemplate {
  return {
    name,
    label: name,
    type: 'TOWN',
    width: 2,
    height: 1,
    spriteWidth: 16,
    spriteHeight: 16,
    tilesets: [''],
    layers: [0],
    tiles: { '0': [0, 0, 0, 0] },
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };
}

function grid(cells: string[][]): MapGridTemplate {
  return {
    name: 'world',
    label: 'World',
    gridWidth: cells[0]?.length ?? 1,
    gridHeight: cells.length,
    mapWidth: 2,
    mapHeight: 1,
    cells,
  };
}

function event(
  id: string,
  vars: Array<Pick<GameEvent['vars'][number], 'id' | 'importFrom'>> = [],
): GameEvent {
  return {
    id,
    title: `${id} title`,
    eventType: 'MODAL',
    icon: '',
    vars: vars.map((variable) => ({ key: '', value: '', ...variable })),
    children: [],
  };
}

function assertMapRenameIsCoherent(
  snapshot: DatabaseSnapshot,
  expectedName: string,
): void {
  const savedMaps = snapshot.maps as unknown as CarcerMapTemplate[];
  const savedGrids = snapshot.mapGrids as unknown as MapGridTemplate[];
  assert.deepEqual(
    savedMaps.map(({ name }) => name),
    [expectedName],
  );
  assert.deepEqual(savedGrids[0]?.cells, [[expectedName]]);
}

function assertEventRenameIsCoherent(snapshot: DatabaseSnapshot): void {
  const savedEvents = snapshot.specialEvents as unknown as GameEvent[];
  const savedMaps = snapshot.maps as unknown as CarcerMapTemplate[];
  const savedCharacters =
    snapshot.characterTemplates as unknown as CharacterTemplate[];
  const savedItems = snapshot.itemTemplates as unknown as ItemTemplate[];

  assert.deepEqual(
    savedEvents.map(({ id }) => id),
    ['consumer', 'new-event', 'unrelated'],
  );
  assert.equal(
    savedEvents
      .flatMap(({ vars }) => vars)
      .some(({ importFrom }) => importFrom === 'old-event'),
    false,
  );
  assert.equal(savedMaps[0]?.eventTriggers[0]?.eventId, 'new-event');
  assert.equal(savedMaps[0]?.eventTriggers[1]?.eventId, 'another-event');
  assert.equal(savedCharacters[0]?.talk?.talkName, 'new-event');
  assert.equal(savedCharacters[1]?.talk?.talkName, 'another-event');
  assert.equal(savedItems[0]?.useSpecialEvent, 'new-event');
  assert.equal(savedItems[1]?.useSpecialEvent, 'another-event');
}
