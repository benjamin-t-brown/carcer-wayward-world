import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import {
  EventRunner,
  parseSpecialEventCollection,
  validateEventRun,
} from '../../src/core/domain/events/index.js';

test('every checked-in event has valid graph/imports and reaches a deterministic first stop', async () => {
  const source = await readFile(
    new URL('../../../src/assets/db/special-events.json', import.meta.url),
    'utf8',
  );
  const raw = JSON.parse(source) as unknown;
  const events = parseSpecialEventCollection(raw);
  const warningCodes: string[] = [];

  for (const event of events) {
    const validation = validateEventRun(event, events);
    assert.equal(
      validation.runnable,
      true,
      `${event.id} must have a structurally runnable graph`,
    );
    assert.deepEqual(
      validation.variablePlan.issues,
      [],
      `${event.id} must resolve all variable imports without conflicts`,
    );
    warningCodes.push(...validation.graphIssues.map(({ code }) => code));

    for (const conditionResult of [false, true]) {
      const runner = new EventRunner(event, events, {
        evaluateCondition: () => conditionResult,
        execute: () => undefined,
      });
      const outcome = runner.start();
      assert.notEqual(
        outcome.kind,
        'error',
        `${event.id} must reach a stop when conditions are ${conditionResult}`,
      );
    }
  }

  assert.equal(events.length, 33);
  assert.deepEqual(
    new Set(warningCodes),
    new Set(['runtime-unsupported-node-type']),
  );
  assert.equal(
    warningCodes.length,
    15,
    'the warnings are the 15 COMMENT nodes',
  );
  assert.equal(JSON.stringify(events), JSON.stringify(raw));
});
