import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import {
  analyzeEventGraph,
  connectionsFromNode,
  EventDocument,
  parseSpecialEventCollection,
  parseSpecialEventRecord,
  SpecialEventParseError,
  type EventChoiceNode,
  type SpecialEventRecord,
} from '../../src/core/domain/events/index.js';

function eventFixture(): SpecialEventRecord {
  return {
    id: 'test_event',
    title: 'Test Event',
    eventType: 'TALK',
    icon: 'portraits0_1',
    vars: [
      {
        id: 'v1',
        key: 'GREETING',
        value: 'hello',
        importFrom: '',
        futureVariableField: { retained: true },
      },
    ],
    children: [
      {
        id: 'root',
        x: 20.5,
        y: 30,
        eventChildType: 'EXEC',
        h: 50,
        p: 'Hello',
        execStr: '',
        next: 'pick',
        autoAdvance: true,
        futureNodeField: ['retained'],
      },
      {
        id: 'pick',
        x: 200,
        y: 30,
        eventChildType: 'CHOICE',
        h: 84,
        text: '',
        choices: [
          {
            text: 'Continue',
            next: 'finish',
            switchText: [
              {
                conditionStr: 'IS(tmp.alt)',
                text: 'Alternative',
                futureAlternateField: 4,
              },
            ],
            futureChoiceField: null,
          },
        ],
      },
      {
        id: 'finish',
        x: 400,
        y: 30,
        eventChildType: 'END',
        h: 60,
        next: '',
      },
    ],
    futureEventField: { version: 2 },
  };
}

function choiceNode(event: SpecialEventRecord, index: number): EventChoiceNode {
  return event.children![index]! as EventChoiceNode;
}

test('parser preserves unknown nested fields, key order, omissions, and ownership', () => {
  const input = eventFixture();
  const sourceJson = JSON.stringify(input);
  const parsed = parseSpecialEventRecord(input);

  input.title = 'Changed outside';
  input.vars![0]!.futureVariableField = { retained: false };
  choiceNode(input, 1).choices![0]!.text = 'Changed outside';

  assert.equal(JSON.stringify(parsed), sourceJson);
  assert.equal('audioInfo' in parsed.children![0]!, false);
  assert.deepEqual(parsed.futureEventField, { version: 2 });
  assert.deepEqual(parsed.vars![0]!.futureVariableField, { retained: true });
});

test('parser validates known nested shapes and reports the exact path', () => {
  const invalid = eventFixture();
  choiceNode(invalid, 1).choices![0]!.switchText![0]!.conditionStr = 4 as never;

  assert.throws(
    () => parseSpecialEventCollection([eventFixture(), invalid]),
    (error: unknown) => {
      assert.ok(error instanceof SpecialEventParseError);
      assert.equal(
        error.path,
        'specialEvents[1].children[1].choices[0].switchText[0].conditionStr',
      );
      return true;
    },
  );
});

test('unknown event and node types remain losslessly representable', () => {
  const fixture = eventFixture();
  fixture.eventType = 'FUTURE_OVERLAY';
  fixture.children!.push({
    id: 'future',
    eventChildType: 'FUTURE_BRANCH',
    links: [{ destination: 'root' }],
    payload: { format: 3 },
  });

  const parsed = parseSpecialEventRecord(fixture);
  const analysis = analyzeEventGraph(parsed);

  assert.deepEqual(parsed, fixture);
  assert.equal(
    analysis.issues.some(
      ({ code, nodeId }) => code === 'unknown-node-type' && nodeId === 'future',
    ),
    true,
  );
});

test('graph analysis enumerates ordered exits and diagnoses broken graphs', () => {
  const fixture = eventFixture();
  fixture.children!.splice(1, 0, {
    id: 'switch',
    eventChildType: 'SWITCH',
    cases: [
      { conditionStr: 'IS(tmp.one)', next: 'finish' },
      { conditionStr: 'IS(tmp.two)', next: 'missing' },
    ],
    defaultNext: 'pick',
  });
  fixture.children!.push({
    id: ' finish ',
    eventChildType: 'COMMENT',
    comment: 'Whitespace duplicate is retained.',
  });

  const analysis = analyzeEventGraph(fixture);
  assert.deepEqual(connectionsFromNode(fixture.children![1]!), [
    {
      sourceId: 'switch',
      targetId: 'finish',
      kind: 'switchCase',
      branchIndex: 0,
    },
    {
      sourceId: 'switch',
      targetId: 'missing',
      kind: 'switchCase',
      branchIndex: 1,
    },
    { sourceId: 'switch', targetId: 'pick', kind: 'defaultNext' },
  ]);
  assert.equal(analysis.nodesById.get('finish')?.length, 1);
  assert.equal(analysis.incomingByNodeId.get('finish')?.length, 2);
  assert.equal(
    analysis.issues.some(({ code }) => code === 'duplicate-node-id'),
    true,
  );
  assert.equal(
    analysis.issues.some(
      ({ code, targetId }) =>
        code === 'missing-target' && targetId === 'missing',
    ),
    true,
  );
  assert.equal(
    analysis.issues.some(
      ({ code, nodeId }) =>
        code === 'runtime-unsupported-node-type' && nodeId === ' finish ',
    ),
    true,
  );
});

test('document edits are explicit and retain unrelated nested data', () => {
  const document = EventDocument.from(eventFixture());
  document.updateHeader({ title: 'Edited title' });
  document.setNodePosition(0, 75, 90);
  document.patchNode(1, { text: 'Make a selection' });
  document.insertNode(
    {
      id: 'note',
      eventChildType: 'COMMENT',
      comment: 'editor only',
      custom: 42,
    },
    2,
  );

  const beforeRemoval = document.analyze();
  assert.equal(beforeRemoval.incomingByNodeId.get('finish')?.length, 1);
  const removed = document.removeNode(3);
  assert.equal(removed.id, 'finish');

  const snapshot = document.snapshot();
  assert.equal(snapshot.title, 'Edited title');
  assert.equal(snapshot.children![0]!.x, 75);
  assert.deepEqual(snapshot.children![0]!.futureNodeField, ['retained']);
  assert.equal(choiceNode(snapshot, 1).choices![0]!.futureChoiceField, null);
  assert.equal(snapshot.children![2]!.custom, 42);
  assert.equal(choiceNode(snapshot, 1).choices![0]!.next, 'finish');
  assert.equal(
    document
      .analyze()
      .issues.some(
        ({ code, targetId }) =>
          code === 'missing-target' && targetId === 'finish',
      ),
    true,
    'deletion must not silently rewrite incoming links',
  );

  snapshot.children![0]!.p = 'external mutation';
  assert.equal(document.snapshot().children![0]!.p, 'Hello');
});

test('the checked-in special-event database round-trips exactly', async () => {
  const source = await readFile(
    new URL('../../../src/assets/db/special-events.json', import.meta.url),
    'utf8',
  );
  const raw = JSON.parse(source) as unknown;
  const records = parseSpecialEventCollection(raw);
  const nodes = records.flatMap((event) => event.children ?? []);
  const typeCounts = new Map<string, number>();
  for (const node of nodes) {
    typeCounts.set(
      node.eventChildType,
      (typeCounts.get(node.eventChildType) ?? 0) + 1,
    );
  }

  assert.equal(records.length, 33);
  assert.deepEqual(Object.fromEntries([...typeCounts].sort()), {
    CHOICE: 72,
    COMMENT: 15,
    END: 42,
    EXEC: 387,
    SWITCH: 38,
  });
  assert.equal(JSON.stringify(records), JSON.stringify(raw));
});
