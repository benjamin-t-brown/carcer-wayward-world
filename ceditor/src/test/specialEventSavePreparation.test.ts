import assert from 'node:assert/strict';
import test from 'node:test';

import type {
  CarcerMapTemplate,
  CharacterTemplate,
  GameEvent,
  ItemTemplate,
} from '../client/types/assets';
import {
  prepareAtomicGameEventRename,
  prepareGameEventsForSave,
} from '../client/utils/specialEventSavePreparation';

test('atomic event rename prepares the event and every reference without mutation', () => {
  const gameEvents: GameEvent[] = [
    event('old-event', [variable('self', 'old-event')]),
    event('consumer', [variable('import', 'old-event')]),
  ];
  const maps = [
    {
      name: ' map-one ',
      width: 4,
      eventTriggers: [{ l: 0, i: 2, eventId: 'old-event' }],
    } as unknown as CarcerMapTemplate,
  ];
  const characters = [
    {
      name: 'speaker',
      talk: { talkName: 'old-event', portraitName: ' portrait ' },
    } as unknown as CharacterTemplate,
  ];
  const items = [
    {
      name: 'letter',
      useSpecialEvent: 'old-event',
      futureItemField: ' keep ',
    } as unknown as ItemTemplate,
  ];
  const before = structuredClone({ gameEvents, maps, characters, items });
  const updated = {
    ...gameEvents[0],
    id: 'new-event',
    title: ' Renamed event ',
  };

  const prepared = prepareAtomicGameEventRename({
    gameEvents,
    maps,
    characters,
    items,
    previousEventId: 'old-event',
    updatedGameEvent: updated,
  });

  assert.deepEqual({ gameEvents, maps, characters, items }, before);
  assert.deepEqual(prepared.renamedReferenceCollections, [
    'maps',
    'characters',
    'items',
  ]);
  assert.equal(
    prepared.gameEvents.some(({ id }) => id === 'old-event'),
    false,
  );
  assert.equal(
    prepared.gameEvents.find(({ id }) => id === 'new-event')?.title,
    'Renamed event',
  );
  assert.equal(
    prepared.gameEvents
      .flatMap(({ vars }) => vars)
      .every(({ importFrom }) => importFrom !== 'old-event'),
    true,
  );
  assert.equal(prepared.maps?.[0].name, 'map-one');
  assert.equal(prepared.maps?.[0].eventTriggers[0].eventId, 'new-event');
  assert.equal(prepared.characters?.[0].talk?.talkName, 'new-event');
  assert.equal(prepared.characters?.[0].talk?.portraitName, 'portrait');
  assert.equal(prepared.items?.[0].useSpecialEvent, 'new-event');
  assert.equal(
    (prepared.items?.[0] as ItemTemplate & { futureItemField: string })
      .futureItemField,
    'keep',
  );
});

test('event save preparation trims and deterministically sorts a clone', () => {
  const input = [event(' z-event '), event('a-event')];
  const before = structuredClone(input);

  const prepared = prepareGameEventsForSave(input);

  assert.deepEqual(
    prepared.map(({ id }) => id),
    ['a-event', 'z-event'],
  );
  assert.deepEqual(input, before);
  assert.notEqual(prepared, input);
  assert.notEqual(prepared[0], input[1]);
});

function event(id: string, vars: GameEvent['vars'] = []): GameEvent {
  return {
    id,
    title: `${id} title`,
    eventType: 'MODAL',
    icon: 'icon',
    vars,
    children: [],
  };
}

function variable(id: string, importFrom: string): GameEvent['vars'][number] {
  return { id, key: '', value: '', importFrom };
}
