import assert from 'node:assert/strict';
import test, { before } from 'node:test';
import {
  GameEventChildType,
  type GameEventChildChoice,
  type GameEventChildComment,
  type GameEventChildEnd,
  type GameEventChildExec,
  type GameEventChildSwitch,
  type SENode,
} from '../client/types/assets';

type SpecialEventModules = {
  Connector: typeof import('../client/special-event-editor/cmpts/Connector').Connector;
  EditorNodeEnd: typeof import('../client/special-event-editor/cmpts/EndNodeComponent').EditorNodeEnd;
  EditorStateSE: typeof import('../client/special-event-editor/seEditorState').EditorStateSE;
  SpecialEventEditorController: typeof import('../client/special-event-editor/SpecialEventEditorController').SpecialEventEditorController;
  breakTextIntoLines: typeof import('../client/special-event-editor/nodeHelpers').breakTextIntoLines;
  distanceToLineSegment: typeof import('../client/special-event-editor/nodeHelpers').distanceToLineSegment;
  getNodeFromWorldCoords: typeof import('../client/special-event-editor/nodeHelpers').getNodeFromWorldCoords;
  screenCoordsToCanvasCoords: typeof import('../client/special-event-editor/nodeHelpers').screenCoordsToCanvasCoords;
  screenToWorldCoords: typeof import('../client/special-event-editor/nodeHelpers').screenToWorldCoords;
  getEditorState: typeof import('../client/special-event-editor/seEditorState').getEditorState;
  seNodeToEditorNode: typeof import('../client/special-event-editor/seEditorState').seNodeToEditorNode;
};

let modules: SpecialEventModules;

before(async () => {
  const stateModule =
    await import('../client/special-event-editor/seEditorState');
  const helperModule =
    await import('../client/special-event-editor/nodeHelpers');
  const connectorModule =
    await import('../client/special-event-editor/cmpts/Connector');
  const endModule =
    await import('../client/special-event-editor/cmpts/EndNodeComponent');
  const controllerModule =
    await import('../client/special-event-editor/SpecialEventEditorController');

  modules = {
    Connector: connectorModule.Connector,
    EditorNodeEnd: endModule.EditorNodeEnd,
    EditorStateSE: stateModule.EditorStateSE,
    SpecialEventEditorController: controllerModule.SpecialEventEditorController,
    breakTextIntoLines: helperModule.breakTextIntoLines,
    distanceToLineSegment: helperModule.distanceToLineSegment,
    getNodeFromWorldCoords: helperModule.getNodeFromWorldCoords,
    screenCoordsToCanvasCoords: helperModule.screenCoordsToCanvasCoords,
    screenToWorldCoords: helperModule.screenToWorldCoords,
    getEditorState: stateModule.getEditorState,
    seNodeToEditorNode: stateModule.seNodeToEditorNode,
  };
});

function createMeasureContext(): CanvasRenderingContext2D {
  return {
    font: '',
    textAlign: 'left',
    textBaseline: 'top',
    measureText: (text: string) => ({ width: text.length * 10 }),
  } as unknown as CanvasRenderingContext2D;
}

function withoutComputedHeight<T extends SENode>(node: T): Omit<T, 'h'> {
  const { h: _height, ...rest } = node;
  return rest;
}

test('text wrapping preserves explicit blank lines and wraps at word boundaries', () => {
  const lines = modules.breakTextIntoLines(
    'one two\n\nthree',
    55,
    12,
    'arial',
    createMeasureContext(),
  );
  assert.deepEqual(lines, ['one', 'two', '', 'three']);
});

test('line-segment distance clamps to endpoints', () => {
  assert.equal(modules.distanceToLineSegment(5, 3, 0, 0, 10, 0), 3);
  assert.equal(modules.distanceToLineSegment(-4, 0, 0, 0, 10, 0), 4);
  assert.equal(modules.distanceToLineSegment(3, 4, 0, 0, 0, 0), 5);
});

test('connectors retain endpoints and expose collision geometry in both directions', () => {
  const forward = modules.Connector.create('from', 'to', 10, 20, 0);
  forward.updatePosition(10, 20, 110, 80);
  assert.deepEqual(forward.getStartPos(), { x: 10, y: 20 });
  assert.deepEqual(forward.getEndPos(), { x: 110, y: 80 });
  assert.equal(forward.segments.length, 4);
  assert.equal(forward.isLineColliding(60, 50), true);
  assert.equal(forward.isLineColliding(200, 200), false);

  const backward = modules.Connector.create('from', 'to', 100, 20, 1);
  backward.updatePosition(100, 20, 50, 80);
  assert.equal(backward.segments.length, 7);
  assert.deepEqual(backward.segments[0], {
    x: 100,
    y: 20,
    color: 'rgba(100, 255, 255, 0.3)',
  });
  assert.deepEqual(backward.getEndPos(), { x: 50, y: 80 });
});

test('screen coordinates reverse the current event-editor transform', () => {
  const controller = new modules.SpecialEventEditorController();
  const state = modules.getEditorState(controller);
  state.translateX = 100;
  state.translateY = -50;
  state.scale = 2;

  const canvas = {
    width: 1000,
    height: 500,
    getBoundingClientRect: () => ({ left: 10, top: 20 }),
  } as unknown as HTMLCanvasElement;

  assert.deepEqual(
    modules.screenCoordsToCanvasCoords(310, 220, canvas),
    [300, 200],
  );
  assert.deepEqual(
    modules.screenToWorldCoords(310, 220, canvas, 8000, 8000, state),
    [3600, 3875],
  );
});

test('node hit testing is inclusive and gives the visually topmost node priority', () => {
  const state = new modules.EditorStateSE();
  const lower = new modules.EditorNodeEnd(
    {
      id: 'lower',
      eventChildType: GameEventChildType.END,
      x: 25,
      y: 30,
      h: 60,
      next: '',
    },
    state,
  );
  const upper = new modules.EditorNodeEnd(
    {
      id: 'upper',
      eventChildType: GameEventChildType.END,
      x: 25,
      y: 30,
      h: 60,
      next: '',
    },
    state,
  );

  assert.equal(lower.isPointInBounds(25, 30), true);
  assert.equal(lower.isPointInBounds(145, 90), true);
  assert.equal(lower.isPointInBounds(146, 90), false);
  assert.equal(modules.getNodeFromWorldCoords(30, 35, [lower, upper]), upper);
});

test('all supported event node subtypes preserve their serialized data', () => {
  const ctx = createMeasureContext();
  const state = new modules.EditorStateSE();
  const inputs: Array<
    | GameEventChildExec
    | GameEventChildSwitch
    | GameEventChildChoice
    | GameEventChildEnd
    | GameEventChildComment
  > = [
    {
      id: 'exec',
      eventChildType: GameEventChildType.EXEC,
      x: 10,
      y: 20,
      h: 999,
      p: 'Narration',
      execStr: 'giveItem("key")',
      next: 'choice',
      autoAdvance: false,
      audioInfo: { audioName: 'voice', volume: 0.75, offset: 125 },
    },
    {
      id: 'switch',
      eventChildType: GameEventChildType.SWITCH,
      x: 30,
      y: 40,
      h: 999,
      defaultNext: 'end',
      cases: [
        { conditionStr: 'hasKey', next: 'choice' },
        { conditionStr: 'isNight', next: 'exec' },
      ],
    },
    {
      id: 'choice',
      eventChildType: GameEventChildType.CHOICE,
      x: 50,
      y: 60,
      h: 999,
      text: 'What now?',
      audioInfo: { audioName: 'prompt', volume: 1, offset: 0 },
      choices: [
        {
          text: 'Open it',
          conditionStr: 'hasKey',
          evalStr: 'consumeKey()',
          next: 'exec',
          prefixText: '> ',
          switchText: [{ conditionStr: 'isNight', text: 'Unlock it' }],
        },
      ],
    },
    {
      id: 'end',
      eventChildType: GameEventChildType.END,
      x: 70,
      y: 80,
      h: 999,
      next: '',
    },
    {
      id: 'comment',
      eventChildType: GameEventChildType.COMMENT,
      x: 90,
      y: 100,
      h: 999,
      comment: 'Designer note',
    },
  ];

  const editorNodes = inputs.map((input) =>
    modules.seNodeToEditorNode(input, state, ctx),
  );
  state.editorNodes = editorNodes;

  for (let i = 0; i < inputs.length; i++) {
    const serialized = editorNodes[i].toSENode();
    assert.deepEqual(
      withoutComputedHeight(serialized),
      withoutComputedHeight(inputs[i]),
    );
    assert.ok(serialized.h > 0);
  }
});
