import assert from 'node:assert/strict';
import test from 'node:test';

import {
  EventRunner,
  replaceEventVariables,
  resolveEventVariables,
  splitEventStatements,
  validateEventRun,
  type EventEvaluationContext,
  type SpecialEventRecord,
} from '../../src/core/domain/events/index.js';

function runnerFixture(eventType = 'TALK'): SpecialEventRecord {
  return {
    id: 'conversation',
    title: 'Conversation',
    eventType,
    icon: 'portrait',
    vars: [{ key: 'NAME', value: 'Ada', importFrom: '' }],
    children: [
      {
        id: 'root',
        eventChildType: 'EXEC',
        p: 'Hello, @NAME.',
        execStr: 'SET(start); WRAP(one; two)\nSET(second)',
        next: 'route',
        autoAdvance: true,
      },
      {
        id: 'route',
        eventChildType: 'SWITCH',
        cases: [
          { conditionStr: 'FIRST(@NAME)', next: 'missing' },
          { conditionStr: 'SECOND', next: 'menu' },
        ],
        defaultNext: 'missing',
      },
      {
        id: 'menu',
        eventChildType: 'CHOICE',
        text: 'Where should we go?',
        choices: [
          {
            text: 'Hidden',
            conditionStr: 'HIDE',
            next: 'missing',
          },
          {
            text: 'Default @NAME',
            prefixText: '1. ',
            conditionStr: 'SHOW(@NAME)',
            evalStr: 'SELECT(@NAME)',
            next: 'done',
            switchText: [
              { conditionStr: 'ALT', text: 'Special route for @NAME' },
            ],
          },
        ],
      },
      { id: 'done', eventChildType: 'END', next: '' },
    ],
  };
}

test('variable planning follows import order, stops cycles, and preserves source', () => {
  const root = runnerFixture();
  root.vars = [
    { key: 'NAME', value: 'Local', importFrom: '' },
    { importFrom: 'shared' },
    { importFrom: 'absent' },
  ];
  const shared: SpecialEventRecord = {
    id: 'shared',
    title: 'Shared variables',
    eventType: 'MODAL',
    icon: '',
    vars: [
      { key: 'NAME', value: 'Imported', importFrom: '' },
      { key: 'PLACE', value: 'Alinea', importFrom: '' },
      { importFrom: 'conversation' },
    ],
    children: [],
  };
  const source = JSON.stringify([root, shared]);

  const plan = resolveEventVariables(root, [shared]);

  assert.deepEqual(plan.visitedEventIds, ['conversation', 'shared']);
  assert.equal(plan.values.get('NAME'), 'Local');
  assert.equal(plan.values.get('PLACE'), 'Alinea');
  assert.equal(
    replaceEventVariables(' @NAME visits @PLACE. ', plan),
    'Local visits Alinea.',
  );
  assert.deepEqual(plan.issues.map(({ code }) => code).sort(), [
    'duplicate-variable-key',
    'import-cycle',
    'missing-import',
  ]);
  assert.equal(JSON.stringify([root, shared]), source);
});

test('statement splitting matches runtime separators outside parentheses', () => {
  assert.deepEqual(
    splitEventStatements(' ONE(); OUTER(a; INNER(b\nc));\n TWO() '),
    ['ONE()', 'OUTER(a; INNER(b\nc))', 'TWO()'],
  );
});

test('runner deterministically executes EXEC, SWITCH, CHOICE, and END', () => {
  const executed: string[] = [];
  const committed: string[][] = [];
  const visitedContexts: EventEvaluationContext[] = [];
  const runner = new EventRunner(runnerFixture(), [], {
    evaluateCondition(expression, context) {
      visitedContexts.push(context);
      if (expression === 'SECOND') return { result: true, onceKeys: ['route'] };
      if (expression === 'SHOW(Ada)') {
        return { result: true, onceKeys: ['choice'] };
      }
      if (expression === 'ALT')
        return { result: true, onceKeys: ['alternate'] };
      return false;
    },
    execute(statement) {
      executed.push(statement);
    },
    commitOnceKeys(keys) {
      committed.push([...keys]);
    },
  });

  const started = runner.start();
  assert.deepEqual(executed, ['SET(start)', 'WRAP(one; two)', 'SET(second)']);
  assert.equal(started.kind, 'choice');
  if (started.kind !== 'choice') return;
  assert.equal(started.steps, 3);
  assert.equal(started.text, 'Hello, Ada.\n\nWhere should we go?');
  assert.deepEqual(started.choices, [
    {
      authoredIndex: 1,
      key: 'menu:1',
      prefix: '1.',
      text: 'Special route for Ada',
      next: 'done',
      onceKeys: ['choice', 'alternate'],
    },
  ]);
  assert.deepEqual(committed, [['route']]);
  assert.equal(
    visitedContexts.every(({ eventId }) => eventId === 'conversation'),
    true,
  );

  const ended = runner.selectChoice(0);
  assert.equal(ended.kind, 'ended');
  assert.equal(ended.nodeId, 'done');
  assert.equal(ended.kind === 'ended' ? ended.text : 'not-ended', '');
  assert.deepEqual(committed, [['route'], ['choice', 'alternate']]);
  assert.deepEqual(executed.at(-1), 'SELECT(Ada)');
});

test('MODAL text stops even when autoAdvance is authored', () => {
  const fixture = runnerFixture('MODAL');
  fixture.children = [
    {
      id: 'root',
      eventChildType: 'EXEC',
      p: 'Read this',
      next: 'done',
      autoAdvance: true,
    },
    { id: 'done', eventChildType: 'END' },
  ];
  const runner = new EventRunner(fixture, [], {
    evaluateCondition: () => false,
    execute: () => undefined,
  });

  assert.deepEqual(runner.start(), {
    kind: 'continue',
    nodeId: 'root',
    text: 'Read this',
    steps: 1,
  });
  assert.deepEqual(runner.continue(), {
    kind: 'ended',
    nodeId: 'done',
    text: 'End.',
    steps: 1,
  });
});

test('runner reports callback errors and never mutates its source event', () => {
  const fixture = runnerFixture();
  const source = JSON.stringify(fixture);
  const runner = new EventRunner(fixture, [], {
    evaluateCondition(expression) {
      if (expression === 'FIRST(Ada)') throw new Error('bad condition');
      return false;
    },
    execute: () => undefined,
  });

  const outcome = runner.start();
  assert.equal(outcome.kind, 'error');
  assert.equal(
    outcome.kind === 'error' ? outcome.error.code : '',
    'condition-error',
  );
  assert.equal(
    outcome.kind === 'error' ? outcome.error.message : '',
    'Condition "FIRST(Ada)" failed: bad condition',
  );
  assert.equal(JSON.stringify(fixture), source);
});

test('execution errors identify both the node and resolved source statement', () => {
  const fixture = runnerFixture();
  const runner = new EventRunner(fixture, [], {
    evaluateCondition: () => false,
    execute(statement) {
      if (statement === 'WRAP(one; two)')
        throw new Error('unsupported command');
    },
  });

  const outcome = runner.start();

  assert.equal(outcome.kind, 'error');
  if (outcome.kind !== 'error') return;
  assert.equal(outcome.nodeId, 'root');
  assert.equal(outcome.error.code, 'execution-error');
  assert.equal(
    outcome.error.message,
    'Statement "WRAP(one; two)" failed: unsupported command',
  );
});

test('runner reports duplicate, missing, unsupported, invalid choice, and step-limit outcomes', () => {
  const callbacks = {
    evaluateCondition: () => true,
    execute: () => undefined,
  };
  const duplicate = runnerFixture();
  duplicate.children = [
    { id: 'root', eventChildType: 'END' },
    { id: 'root', eventChildType: 'END' },
  ];
  const duplicateOutcome = new EventRunner(duplicate, [], callbacks).start();
  assert.equal(
    duplicateOutcome.kind === 'error' ? duplicateOutcome.error.code : '',
    'duplicate-node',
  );

  const missing = runnerFixture();
  missing.children = [
    { id: 'root', eventChildType: 'EXEC', next: 'absent', autoAdvance: true },
  ];
  const missingOutcome = new EventRunner(missing, [], callbacks).start();
  assert.equal(
    missingOutcome.kind === 'error' ? missingOutcome.error.code : '',
    'missing-node',
  );

  const unsupported = runnerFixture();
  unsupported.children = [{ id: 'root', eventChildType: 'FUTURE' }];
  const unsupportedOutcome = new EventRunner(
    unsupported,
    [],
    callbacks,
  ).start();
  assert.equal(
    unsupportedOutcome.kind === 'error' ? unsupportedOutcome.error.code : '',
    'unsupported-node',
  );

  const choiceFixture = runnerFixture();
  choiceFixture.children = [
    {
      id: 'root',
      eventChildType: 'CHOICE',
      choices: [{ text: 'Go', next: 'done' }],
    },
    { id: 'done', eventChildType: 'END' },
  ];
  const choiceRunner = new EventRunner(choiceFixture, [], callbacks);
  const choiceOutcome = choiceRunner.start();
  assert.equal(choiceOutcome.kind, 'choice');
  const invalidChoice = choiceRunner.selectChoice(50);
  assert.equal(
    invalidChoice.kind === 'error' ? invalidChoice.error.code : '',
    'invalid-choice',
  );

  const loop = runnerFixture();
  loop.children = [
    {
      id: 'root',
      eventChildType: 'SWITCH',
      cases: [],
      defaultNext: 'root',
    },
  ];
  const loopOutcome = new EventRunner(loop, [], callbacks, {
    maxSteps: 4,
  }).start();
  assert.equal(
    loopOutcome.kind === 'error' ? loopOutcome.error.code : '',
    'step-limit',
  );
  assert.equal(loopOutcome.steps, 4);
});

test('run validation combines graph and variable-import diagnostics', () => {
  const fixture = runnerFixture();
  fixture.vars!.push({ importFrom: 'missing-event' });
  fixture.children!.push({
    id: 'broken',
    eventChildType: 'EXEC',
    next: 'missing-node',
  });

  const validation = validateEventRun(fixture);

  assert.equal(validation.runnable, false);
  assert.equal(
    validation.graphIssues.some(({ code }) => code === 'missing-target'),
    true,
  );
  assert.equal(
    validation.variablePlan.issues.some(
      ({ code }) => code === 'missing-import',
    ),
    true,
  );
});
