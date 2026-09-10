import assert from 'node:assert/strict';
import test from 'node:test';
import {
  itemAtSourceIndex,
  replaceAtSourceIndex,
  sourceIndexFromVisibleIndex,
  visibleIndexFromSourceIndex,
} from '../client/utils/editorListSelection';

const source = [
  { name: 'apple' },
  { name: 'axe' },
  { name: 'bread' },
  { name: 'bow' },
];
const visible = [source[2], source[3]];

test('translates a filtered-list click to the source-list index', () => {
  assert.equal(sourceIndexFromVisibleIndex(source, visible, 0), 2);
  assert.equal(sourceIndexFromVisibleIndex(source, visible, 1), 3);
  assert.equal(sourceIndexFromVisibleIndex(source, visible, 2), -1);
});

test('translates a source selection to its visible position', () => {
  assert.equal(visibleIndexFromSourceIndex(source, visible, 3), 1);
  assert.equal(visibleIndexFromSourceIndex(source, visible, 0), null);
  assert.equal(visibleIndexFromSourceIndex(source, visible, -1), null);
});

test('reads and replaces the source selection without using a filtered index', () => {
  assert.equal(itemAtSourceIndex(source, 2)?.name, 'bread');

  const next = replaceAtSourceIndex(source, 2, { name: 'brioche' });
  assert.deepEqual(
    next.map((item) => item.name),
    ['apple', 'axe', 'brioche', 'bow'],
  );
  assert.equal(source[2].name, 'bread');
});

test('an invalid source selection does not replace another item', () => {
  const next = replaceAtSourceIndex(source, -1, { name: 'wrong' });
  assert.deepEqual(next, source);
  assert.notEqual(next, source);
});
