import assert from 'node:assert/strict';
import test from 'node:test';

import { DatabaseConflictError } from '../../src/core/database/index.js';
import {
  databasePageErrorMessage,
  dirtyAssetLabels,
  isSaveShortcut,
  summarizeValidation,
} from '../../src/core/ui/index.js';

test('dirty asset labels follow registry order', () => {
  assert.deepEqual(
    dirtyAssetLabels(new Set(['specialEvents', 'items', 'abilities'])),
    ['Abilities', 'Items', 'Special Events'],
  );
});

test('recognizes cross-platform Save All shortcuts', () => {
  assert.equal(
    isSaveShortcut({ key: 's', metaKey: true, ctrlKey: false, altKey: false }),
    true,
  );
  assert.equal(
    isSaveShortcut({ key: 'S', metaKey: false, ctrlKey: true, altKey: false }),
    true,
  );
  assert.equal(
    isSaveShortcut({ key: 's', metaKey: false, ctrlKey: false, altKey: false }),
    false,
  );
  assert.equal(
    isSaveShortcut({ key: 's', metaKey: true, ctrlKey: false, altKey: true }),
    false,
  );
});

test('summarizes validation errors and warnings', () => {
  const error = {
    severity: 'error' as const,
    code: 'bad',
    message: 'Bad value',
    path: 'items[0]',
  };
  const warning = {
    severity: 'warning' as const,
    code: 'maybe',
    message: 'Maybe bad',
    path: 'maps[0]',
  };

  assert.deepEqual(
    summarizeValidation({
      valid: false,
      issues: [error, warning],
      errors: [error],
      warnings: [warning],
    }),
    { errorCount: 1, warningCount: 1, text: '1 error, 1 warning' },
  );
});

test('gives save conflicts an actionable message', () => {
  assert.match(
    databasePageErrorMessage(new DatabaseConflictError('stale revision')),
    /Reload the page/,
  );
  assert.equal(databasePageErrorMessage(new Error('offline')), 'offline');
});
