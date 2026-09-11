import assert from 'node:assert/strict';
import test from 'node:test';

import {
  cloneSpecialEventDocument,
  serializeSpecialEventDocument,
  serializeSpecialEventNodes,
  snapshotSpecialEventDocuments,
  type SpecialEventNodeSerializer,
} from '../client/special-event-editor/specialEventDocument';
import {
  GameEventChildType,
  type GameEvent,
  type GameEventChildChoice,
} from '../client/types/assets';

test('document cloning detaches mutable editor input from saved assets', () => {
  const savedAsset = event('active', 'Stored choice');
  const editorInput = cloneSpecialEventDocument(savedAsset);

  editorInput.vars[0].value = 'edited value';
  (
    editorInput.children[0] as GameEventChildChoice
  ).choices[0].switchText![0].text = 'Edited alternate';

  assert.equal(savedAsset.vars[0].value, 'original value');
  assert.equal(
    (savedAsset.children[0] as GameEventChildChoice).choices[0].switchText![0]
      .text,
    'Alternate choice',
  );
});

test('node serialization deeply detaches nested editor-node data', () => {
  const sourceNode = choiceNode('edited-child', 'Edited choice');
  const serializer: SpecialEventNodeSerializer = {
    toSENode: () => sourceNode,
  };

  const serialized = serializeSpecialEventNodes([serializer]);
  const serializedChoice = serialized[0] as GameEventChildChoice;
  serializedChoice.choices[0].text = 'Changed after serialization';
  serializedChoice.choices[0].switchText![0].text = 'Changed nested text';

  assert.equal(sourceNode.choices[0].text, 'Edited choice');
  assert.equal(sourceNode.choices[0].switchText![0].text, 'Alternate choice');
});

test('event serialization preserves metadata and replaces children without aliases', () => {
  const source = event('active', 'Original child');
  const sourceBefore = structuredClone(source);
  const serializedNode = choiceNode('edited-child', 'Edited choice');

  const document = serializeSpecialEventDocument(source, [
    { toSENode: () => serializedNode },
  ]);

  assert.deepEqual(source, sourceBefore);
  assert.equal(document.children[0].id, 'edited-child');
  assert.notEqual(document, source);
  assert.notEqual(document.vars, source.vars);
  assert.notEqual(document.children[0], serializedNode);

  document.vars[0].value = 'changed';
  (document.children[0] as GameEventChildChoice).choices[0].text = 'changed';
  assert.equal(source.vars[0].value, 'original value');
  assert.equal(serializedNode.choices[0].text, 'Edited choice');
});

test('snapshot gives save and current-event consumers identical detached content', () => {
  const events = [event('active', 'Stored child'), event('other', 'Other')];
  const before = structuredClone(events);
  const editedNode = choiceNode('edited-child', 'Unsaved canvas edit');

  const snapshot = snapshotSpecialEventDocuments(events, {
    eventId: 'active',
    nodes: [{ toSENode: () => editedNode }],
  });
  const currentInCollection = snapshot.gameEvents.find(
    ({ id }) => id === 'active',
  );

  assert.deepEqual(events, before);
  assert.deepEqual(snapshot.currentGameEvent, currentInCollection);
  assert.notEqual(snapshot.currentGameEvent, currentInCollection);
  assert.notEqual(snapshot.gameEvents, events);
  assert.notEqual(snapshot.gameEvents[1], events[1]);
  assert.notEqual(snapshot.gameEvents[1].children[0], events[1].children[0]);

  (
    snapshot.currentGameEvent!.children[0] as GameEventChildChoice
  ).choices[0].text = 'Runner mutation';
  assert.equal(
    (currentInCollection!.children[0] as GameEventChildChoice).choices[0].text,
    'Unsaved canvas edit',
  );
  assert.equal(editedNode.choices[0].text, 'Unsaved canvas edit');
});

test('snapshot safely clones assets when there is no matching active document', () => {
  const events = [event('first', 'Stored child')];
  let serialized = false;

  const snapshot = snapshotSpecialEventDocuments(events, {
    eventId: 'missing',
    nodes: [
      {
        toSENode: () => {
          serialized = true;
          return choiceNode('unused', 'Unused');
        },
      },
    ],
  });

  assert.equal(serialized, false);
  assert.equal(snapshot.currentGameEvent, undefined);
  assert.deepEqual(snapshot.gameEvents, events);
  assert.notEqual(snapshot.gameEvents, events);
  assert.notEqual(snapshot.gameEvents[0], events[0]);
});

function event(id: string, choiceText: string): GameEvent {
  return {
    id,
    title: `${id} title`,
    eventType: 'MODAL',
    icon: 'special-event-icon',
    vars: [
      {
        id: `${id}-var`,
        key: 'key',
        value: 'original value',
        importFrom: '',
      },
    ],
    children: [choiceNode(`${id}-child`, choiceText)],
  };
}

function choiceNode(id: string, choiceText: string): GameEventChildChoice {
  return {
    id,
    eventChildType: GameEventChildType.CHOICE,
    x: 10,
    y: 20,
    h: 100,
    text: 'Choose',
    choices: [
      {
        text: choiceText,
        next: '',
        switchText: [{ conditionStr: 'isAlternate', text: 'Alternate choice' }],
      },
    ],
  };
}
