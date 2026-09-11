import assert from 'node:assert/strict';
import test from 'node:test';

import { SpecialEventEditorController } from '../client/special-event-editor/SpecialEventEditorController';
import { EditorNodeChoice } from '../client/special-event-editor/cmpts/ChoiceNodeComponent';
import {
  createEditorNodesForGameEvent,
  restoreEditorStateForGameEvent,
  saveEditorStateForGameEvent,
} from '../client/special-event-editor/seEditorState';
import {
  GameEventChildType,
  type GameEvent,
  type GameEventChildChoice,
  type GameEventChildEnd,
} from '../client/types/assets';

class TrackingTarget extends EventTarget {
  private readonly listenerCounts = new Map<string, number>();

  override addEventListener(
    type: string,
    callback: EventListenerOrEventListenerObject | null,
    options?: AddEventListenerOptions | boolean,
  ): void {
    super.addEventListener(type, callback, options);
    this.listenerCounts.set(type, (this.listenerCounts.get(type) ?? 0) + 1);
  }

  override removeEventListener(
    type: string,
    callback: EventListenerOrEventListenerObject | null,
    options?: EventListenerOptions | boolean,
  ): void {
    super.removeEventListener(type, callback, options);
    this.listenerCounts.set(type, (this.listenerCounts.get(type) ?? 0) - 1);
  }

  count(type: string): number {
    return this.listenerCounts.get(type) ?? 0;
  }
}

function createMeasureContext(): CanvasRenderingContext2D {
  return {
    font: '',
    textAlign: 'left',
    textBaseline: 'top',
    measureText: (text: string) => ({ width: text.length * 10 }),
  } as unknown as CanvasRenderingContext2D;
}

class FakeCanvas extends TrackingTarget {
  tabIndex = -1;
  focusCount = 0;
  capturedPointerIds: number[] = [];
  releasedPointerIds: number[] = [];

  hasAttribute(name: string): boolean {
    return name === 'tabindex' && this.tabIndex >= 0;
  }

  focus(): void {
    this.focusCount++;
  }

  setPointerCapture(pointerId: number): void {
    this.capturedPointerIds.push(pointerId);
  }

  hasPointerCapture(pointerId: number): boolean {
    return this.capturedPointerIds.includes(pointerId);
  }

  releasePointerCapture(pointerId: number): void {
    this.releasedPointerIds.push(pointerId);
  }

  getContext(): CanvasRenderingContext2D {
    return createMeasureContext();
  }
}

function pointerEvent(type: string, pointerId: number): Event {
  const event = new Event(type);
  Object.defineProperty(event, 'pointerId', { value: pointerId });
  return event;
}

test('controllers isolate editor, clipboard, viewport, and input timing state', () => {
  const first = new SpecialEventEditorController();
  const second = new SpecialEventEditorController();

  first.updateSilent({
    gameEventId: 'first',
    translateX: 25,
    translateY: -10,
    scale: 2,
    copiedNodes: [
      {
        seNode: {
          id: 'copied',
          eventChildType: GameEventChildType.END,
          x: 1,
          y: 2,
          h: 20,
          next: '',
        } as GameEventChildEnd,
        offsetX: 3,
        offsetY: 4,
      },
    ],
  });
  first.state.selectedNodeIds.add('node');
  first.lastClickTime = 123;
  first.linkingEmptyClickPending = true;

  assert.equal(second.state.gameEventId, undefined);
  assert.deepEqual(
    [second.state.translateX, second.state.translateY, second.state.scale],
    [0, 0, 1],
  );
  assert.equal(second.state.copiedNodes, null);
  assert.deepEqual([...second.state.selectedNodeIds], []);
  assert.equal(second.lastClickTime, 0);
  assert.equal(second.linkingEmptyClickPending, false);
});

test('updates notify subscribers and validation without a window callback', () => {
  const controller = new SpecialEventEditorController();
  let notifications = 0;
  const unsubscribe = controller.subscribe(() => notifications++);

  controller.update({ translateX: 12 });
  assert.equal(controller.state.translateX, 12);
  assert.equal(controller.state.shouldValidate, true);
  assert.equal(notifications, 1);

  controller.state.shouldValidate = false;
  controller.update({ translateY: 8 }, false);
  assert.equal(controller.state.shouldValidate, false);
  assert.equal(notifications, 2);

  controller.updateSilent({ scale: 3 });
  assert.equal(controller.state.scale, 3);
  assert.equal(notifications, 2);

  controller.markValidationRequired();
  assert.equal(controller.state.shouldValidate, true);
  assert.equal(notifications, 3);
  unsubscribe();
});

test('attach scopes pointer input to the canvas and keyboard input to focus', () => {
  const keyboard = new TrackingTarget();
  const canvas = new FakeCanvas();
  const documentTarget = {
    activeElement: null as unknown,
    querySelector: () => null,
  };
  const controller = new SpecialEventEditorController({
    keyboardTarget: keyboard,
    documentTarget: documentTarget as Pick<
      Document,
      'activeElement' | 'querySelector'
    >,
  });
  let downs = 0;
  let moves = 0;
  let ups = 0;
  let keys = 0;
  const attachment = {
    canvas: canvas as unknown as HTMLCanvasElement,
    handlers: {
      pointerdown: () => downs++,
      pointermove: () => moves++,
      pointerup: () => ups++,
      keydown: () => keys++,
    },
  };

  controller.attach(attachment);
  controller.attach(attachment);
  assert.equal(canvas.count('pointerdown'), 1);
  assert.equal(canvas.count('pointermove'), 1);
  assert.equal(keyboard.count('keydown'), 1);
  assert.equal(canvas.tabIndex, 0);

  keyboard.dispatchEvent(new Event('keydown'));
  assert.equal(keys, 0);
  documentTarget.activeElement = canvas;
  keyboard.dispatchEvent(new Event('keydown'));
  assert.equal(keys, 1);

  canvas.dispatchEvent(pointerEvent('pointerdown', 7));
  canvas.dispatchEvent(pointerEvent('pointermove', 7));
  canvas.dispatchEvent(pointerEvent('pointerup', 7));
  assert.equal(canvas.focusCount, 1);
  assert.deepEqual(canvas.capturedPointerIds, [7]);
  assert.deepEqual(canvas.releasedPointerIds, [7]);
  assert.deepEqual([downs, moves, ups], [1, 1, 1]);

  controller.detach();
  assert.equal(canvas.count('pointerdown'), 0);
  assert.equal(canvas.count('wheel'), 0);
  assert.equal(keyboard.count('keydown'), 0);
});

test('detach aborts partial gestures and destroy clears session-owned data', () => {
  const canvas = new FakeCanvas();
  const controller = new SpecialEventEditorController();
  controller.attach({
    canvas: canvas as unknown as HTMLCanvasElement,
    handlers: {},
  });
  controller.state.isDragging = true;
  controller.state.isDraggingNode = true;
  controller.state.draggedNodeId = 'dragged';
  controller.state.isSelecting = true;
  controller.state.selectionRect = {
    startX: 1,
    startY: 2,
    endX: 3,
    endY: 4,
  };
  controller.state.editorSaveStates.set('saved', {
    serializedNodes: [],
    gameEventTransform: { translateX: 0, translateY: 0, scale: 1 },
  });
  canvas.dispatchEvent(pointerEvent('pointerdown', 9));

  controller.detach();
  assert.equal(controller.state.isDragging, false);
  assert.equal(controller.state.isDraggingNode, false);
  assert.equal(controller.state.draggedNodeId, null);
  assert.equal(controller.state.isSelecting, false);
  assert.equal(controller.state.selectionRect, null);
  assert.deepEqual(canvas.releasedPointerIds, [9]);

  controller.destroy();
  controller.destroy();
  assert.equal(controller.isDestroyed, true);
  assert.equal(controller.state.editorSaveStates.size, 0);
  assert.deepEqual(controller.state.editorNodes, []);
  assert.throws(
    () =>
      controller.attach({
        canvas: canvas as unknown as HTMLCanvasElement,
        handlers: {},
      }),
    /destroyed/,
  );
});

test('starting another document clears transient state but keeps the clipboard', () => {
  const controller = new SpecialEventEditorController();
  const copiedNodes = [
    {
      seNode: {
        id: 'copied',
        eventChildType: GameEventChildType.END,
        x: 1,
        y: 2,
        h: 20,
        next: '',
      } as GameEventChildEnd,
      offsetX: 0,
      offsetY: 0,
    },
  ];
  controller.state.copiedNodes = copiedNodes;
  controller.state.selectedNodeIds.add('old-node');
  controller.state.hoveredNodeId = 'old-node';
  controller.state.linking = {
    isLinking: true,
    sourceNodeId: 'old-node',
    exitIndex: 2,
  };
  controller.state.runnerErrors = [{ message: 'old error', nodeId: 'old' }];
  controller.lastClickTime = 123;
  controller.lastClickNodeId = 'old-node';

  controller.resetDocumentInteraction();

  assert.deepEqual([...controller.state.selectedNodeIds], []);
  assert.equal(controller.state.hoveredNodeId, undefined);
  assert.deepEqual(controller.state.linking, {
    isLinking: false,
    sourceNodeId: '',
    exitIndex: 0,
  });
  assert.deepEqual(controller.state.runnerErrors, []);
  assert.equal(controller.lastClickTime, 0);
  assert.equal(controller.lastClickNodeId, null);
  assert.equal(controller.state.copiedNodes, copiedNodes);
});

test('saved events restore rebuilt nodes without retaining mutable aliases', () => {
  const canvas = new FakeCanvas() as unknown as HTMLCanvasElement;
  const controller = new SpecialEventEditorController();
  const gameEvent: GameEvent = {
    id: 'choice-event',
    title: 'Choice',
    eventType: 'MODAL',
    icon: '',
    vars: [],
    children: [
      {
        id: 'choice',
        eventChildType: GameEventChildType.CHOICE,
        x: 10,
        y: 20,
        h: 100,
        text: 'Choose',
        choices: [
          {
            text: 'Original',
            conditionStr: '',
            evalStr: '',
            next: '',
            prefixText: '',
            switchText: [{ conditionStr: 'night', text: 'Night choice' }],
          },
        ],
      } as GameEventChildChoice,
    ],
  };
  controller.state.gameEventId = gameEvent.id;
  controller.state.editorNodes = createEditorNodesForGameEvent(
    controller,
    gameEvent,
    canvas,
  );
  const originalEditorNode = controller.state.editorNodes[0];
  const choiceNode = originalEditorNode as EditorNodeChoice;

  choiceNode.choices[0].text = 'Unsaved mutation';
  const sourceChoice = gameEvent.children[0] as GameEventChildChoice;
  assert.equal(sourceChoice.choices[0].text, 'Original');

  choiceNode.choices[0].text = 'Saved value';
  saveEditorStateForGameEvent(controller, gameEvent.id);
  choiceNode.x = 999;
  choiceNode.choices[0].text = 'Later mutation';
  choiceNode.choices[0].switchText![0].text = 'Later nested mutation';

  assert.equal(
    restoreEditorStateForGameEvent(controller, gameEvent.id, canvas),
    true,
  );
  const restored = controller.state.editorNodes[0] as EditorNodeChoice;
  assert.notEqual(restored, originalEditorNode);
  assert.equal(restored.x, 10);
  assert.equal(restored.choices[0].text, 'Saved value');
  assert.equal(restored.choices[0].switchText![0].text, 'Night choice');
});
