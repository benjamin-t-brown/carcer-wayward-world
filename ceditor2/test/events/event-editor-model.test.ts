import assert from 'node:assert/strict';
import test from 'node:test';

import type {
  EventExecNode,
  EventNode,
  EventSwitchNode,
  SpecialEventRecord,
} from '../../src/core/domain/events/index.js';
import {
  cloneSpecialEvent,
  copyEventNodes,
  createEventNode,
  createSpecialEvent,
  createUniqueEventId,
  eventIdFromUrl,
  findDeepLinkedEvent,
  matchesEventSearch,
  pasteEventNodes,
  withEventSelection,
} from '../../src/apps/events/editorModel.js';
import {
  fitEventViewport,
  hitTestEventNode,
  indexesInWorldRectangle,
  screenToWorld,
  worldToScreen,
} from '../../src/apps/events/graphGeometry.js';

function events(): SpecialEventRecord[] {
  return [
    {
      id: 'intro_talk',
      title: 'Introduction',
      eventType: 'TALK',
      icon: 'portrait_1',
      children: [],
    },
    {
      id: 'door_modal',
      title: 'Locked Door',
      eventType: 'MODAL',
      icon: '',
      children: [],
    },
  ];
}

test('event search and deep links use stable persisted IDs', () => {
  const records = events();
  assert.equal(matchesEventSearch(records[0]!, 'INTRO'), true);
  assert.equal(matchesEventSearch(records[1]!, 'talk'), false);
  assert.equal(
    findDeepLinkedEvent(
      records,
      new URL('https://editor/events?event=door_modal'),
    ),
    1,
  );
  assert.equal(
    eventIdFromUrl(new URL('https://editor/events#intro_talk')),
    'intro_talk',
  );

  const selected = withEventSelection(
    new URL('https://editor/events?selected=old&x=1'),
    'intro_talk',
  );
  assert.equal(selected.searchParams.get('event'), 'intro_talk');
  assert.equal(selected.searchParams.get('selected'), null);
  assert.equal(selected.searchParams.get('x'), '1');
});

test('new and cloned events are complete while retaining custom source fields', () => {
  const created = createSpecialEvent('new_event');
  assert.equal(created.children?.[0]?.id, 'root');
  assert.equal(created.children?.[0]?.eventChildType, 'EXEC');
  assert.equal(
    createUniqueEventId('new_event', ['new_event', 'new_event_2']),
    'new_event_3',
  );

  const source: SpecialEventRecord = {
    ...events()[0]!,
    future: { retained: true },
  };
  const cloned = cloneSpecialEvent(source, 'intro_copy');
  assert.equal(cloned.id, 'intro_copy');
  assert.equal(cloned.title, 'Introduction (copy)');
  assert.deepEqual(cloned.future, { retained: true });
  assert.notEqual(cloned, source);
});

test('copy and paste preserve internal graph links and external links', () => {
  const nodes: EventNode[] = [
    createEventNode('EXEC', 'one', 10, 20),
    createEventNode('SWITCH', 'two', 260, 80),
    createEventNode('END', 'outside', 600, 80),
  ];
  (nodes[0]! as EventExecNode).next = 'two';
  (nodes[1]! as EventSwitchNode).defaultNext = 'outside';
  (nodes[1]! as EventSwitchNode).cases = [
    { conditionStr: 'YES()', next: 'one' },
  ];
  const clipboard = copyEventNodes(nodes, new Set([0, 1]));
  assert.ok(clipboard);

  const pasted = pasteEventNodes(
    clipboard,
    nodes.map(({ id }) => id),
    100,
    200,
  );
  assert.deepEqual(
    pasted.map(({ id }) => id),
    ['one_copy', 'two_copy'],
  );
  assert.equal((pasted[0]! as EventExecNode).next, 'two_copy');
  assert.equal((pasted[1]! as EventSwitchNode).defaultNext, 'outside');
  assert.deepEqual((pasted[1]! as EventSwitchNode).cases, [
    { conditionStr: 'YES()', next: 'one_copy' },
  ]);
  assert.deepEqual([pasted[0]!.x, pasted[0]!.y], [100, 200]);
});

test('event graph geometry supports reverse-order hits, box selection, fit, and transforms', () => {
  const nodes: EventNode[] = [
    createEventNode('EXEC', 'behind', 10, 20),
    createEventNode('END', 'front', 20, 30),
    createEventNode('END', 'far', 500, 300),
  ];
  assert.equal(hitTestEventNode(nodes, { x: 30, y: 40 }), 1);
  assert.deepEqual(
    [...indexesInWorldRectangle(nodes, { x: 0, y: 0 }, { x: 245, y: 100 })],
    [0, 1],
  );

  const viewport = fitEventViewport(nodes, 1000, 600);
  assert.ok(viewport.scale > 0);
  const point = { x: 123, y: 456 };
  const roundTrip = screenToWorld(worldToScreen(point, viewport), viewport);
  assert.ok(Math.abs(roundTrip.x - point.x) < 1e-9);
  assert.ok(Math.abs(roundTrip.y - point.y) < 1e-9);
});
