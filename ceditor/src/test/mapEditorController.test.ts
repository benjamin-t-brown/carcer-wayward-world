import assert from 'node:assert/strict';
import test from 'node:test';

import { MapEditorController } from '../client/tile-editor/MapEditorController';
import {
  clearAllSelectedTiles,
  setSoleSelectedTile,
} from '../client/tile-editor/editorState';
import type { PaintAction } from '../client/tile-editor/paintTools';

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
    (globalThis.document as { activeElement: unknown }).activeElement = this;
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
}

class RafHarness {
  private nextId = 1;
  readonly callbacks = new Map<number, FrameRequestCallback>();
  readonly cancelled: number[] = [];

  request = (callback: FrameRequestCallback): number => {
    const id = this.nextId++;
    this.callbacks.set(id, callback);
    return id;
  };

  cancel = (id: number): void => {
    this.cancelled.push(id);
    this.callbacks.delete(id);
  };

  run(id: number, timestamp: number): void {
    const callback = this.callbacks.get(id);
    assert.ok(callback, `missing RAF callback ${id}`);
    this.callbacks.delete(id);
    callback(timestamp);
  }
}

Object.defineProperty(globalThis, 'document', {
  configurable: true,
  value: { activeElement: null },
});

function pointerEvent(type: string, pointerId: number): Event {
  const event = new Event(type);
  Object.defineProperty(event, 'pointerId', { value: pointerId });
  return event;
}

test('controller attach is idempotent and scopes pointer input to its canvas', () => {
  const keyboard = new TrackingTarget();
  const resize = new TrackingTarget();
  const canvas = new FakeCanvas();
  let downs = 0;
  let moves = 0;
  let ups = 0;
  let cancels = 0;
  const attachment = {
    canvas: canvas as unknown as HTMLCanvasElement,
    handlers: {
      pointerdown: () => downs++,
      pointermove: () => moves++,
      pointerup: () => ups++,
      pointercancel: () => cancels++,
    },
  };
  const controller = new MapEditorController({
    keyboardTarget: keyboard,
    resizeTarget: resize,
  });

  controller.attach(attachment);
  controller.attach(attachment);
  assert.equal(canvas.count('pointerdown'), 1);
  assert.equal(canvas.count('pointermove'), 1);
  assert.equal(canvas.count('focus'), 1);
  assert.equal(canvas.count('blur'), 1);
  assert.equal(keyboard.count('keydown'), 1);
  assert.equal(resize.count('resize'), 1);
  assert.equal(canvas.tabIndex, 0);

  canvas.dispatchEvent(pointerEvent('pointerdown', 17));
  canvas.dispatchEvent(pointerEvent('pointermove', 17));
  canvas.dispatchEvent(pointerEvent('pointerup', 17));
  canvas.dispatchEvent(pointerEvent('pointercancel', 18));
  assert.equal(canvas.focusCount, 1);
  assert.deepEqual(canvas.capturedPointerIds, [17]);
  assert.deepEqual(canvas.releasedPointerIds, [17]);
  assert.equal(downs, 1);
  assert.equal(moves, 1);
  assert.equal(ups, 1);
  assert.equal(cancels, 1);

  controller.detach();
  controller.detach();
  assert.equal(controller.isAttached, false);
  assert.equal(canvas.count('pointerdown'), 0);
  assert.equal(canvas.count('pointermove'), 0);
  assert.equal(canvas.count('focus'), 0);
  assert.equal(canvas.count('blur'), 0);
  assert.equal(keyboard.count('keydown'), 0);
  assert.equal(resize.count('resize'), 0);
});

test('continuous rendering has one RAF owner and stop is idempotent', () => {
  const raf = new RafHarness();
  const frames: Array<[number, number]> = [];
  const controller = new MapEditorController({
    requestAnimationFrame: raf.request,
    cancelAnimationFrame: raf.cancel,
  });

  controller.start((timestamp, elapsed) => frames.push([timestamp, elapsed]));
  controller.start(() =>
    assert.fail('a second start must not replace the loop'),
  );
  assert.equal(controller.isRunning, true);
  assert.equal(raf.callbacks.size, 1);

  raf.run(1, 100);
  assert.deepEqual(frames, [[100, 0]]);
  assert.equal(raf.callbacks.size, 1);
  raf.run(2, 116);
  assert.deepEqual(frames, [
    [100, 0],
    [116, 16],
  ]);

  controller.stop();
  controller.stop();
  assert.equal(controller.isRunning, false);
  assert.equal(raf.callbacks.size, 0);
  assert.deepEqual(raf.cancelled, [3]);
});

test('destroy removes every listener, frame, action, and subscriber once', () => {
  const keyboard = new TrackingTarget();
  const resize = new TrackingTarget();
  const canvas = new FakeCanvas();
  const raf = new RafHarness();
  const controller = new MapEditorController({
    keyboardTarget: keyboard,
    resizeTarget: resize,
    requestAnimationFrame: raf.request,
    cancelAnimationFrame: raf.cancel,
  });
  let notifications = 0;
  controller.subscribe(() => notifications++);
  controller.attach({
    canvas: canvas as unknown as HTMLCanvasElement,
    handlers: {},
  });
  controller.setCurrentAction({ type: 'DRAW' } as unknown as PaintAction);
  controller.start(() => {});
  controller.update({ showGrid: false });
  assert.equal(notifications, 1);

  controller.destroy();
  controller.destroy();
  controller.notify();

  assert.equal(controller.isDestroyed, true);
  assert.equal(controller.isAttached, false);
  assert.equal(controller.isRunning, false);
  assert.equal(controller.getCurrentAction(), null);
  assert.equal(notifications, 1);
  assert.equal(raf.callbacks.size, 0);
  assert.equal(canvas.count('pointerdown'), 0);
  assert.equal(canvas.count('wheel'), 0);
  assert.equal(canvas.count('focus'), 0);
  assert.equal(canvas.count('blur'), 0);
  assert.equal(keyboard.count('keydown'), 0);
  assert.equal(keyboard.count('keyup'), 0);
  assert.equal(resize.count('resize'), 0);
  assert.throws(
    () =>
      controller.attach({
        canvas: canvas as unknown as HTMLCanvasElement,
        handlers: {},
      }),
    /destroyed/,
  );
  assert.throws(() => controller.start(() => {}), /destroyed/);
});

test('two controllers share no selection, viewport, clipboard, undo, action, or revision', () => {
  const first = new MapEditorController();
  const second = new MapEditorController();
  const firstMap = first.ensureMap('same-name');
  const secondMap = second.ensureMap('same-name');
  const action = { type: 'DRAW' } as unknown as PaintAction;

  first.update({
    selectedMapName: 'same-name',
    selectedTileIndexInTileset: 12,
    selectedTilesetName: 'terrain',
    rectCloneBrushTiles: [
      {
        xOffset: 1,
        yOffset: 2,
        originalTile: {
          ref: { tilesetName: 'terrain', tileId: 3 } as never,
        },
      },
    ],
  });
  first.input.translateX = 40;
  first.input.translateY = -20;
  first.input.scale = 3;
  firstMap.selectedTileInd = 7;
  firstMap.undoHistory.push(action);
  firstMap.undoIndex = 0;
  first.setCurrentAction(action);
  first.bumpMapRevision('same-name');

  assert.equal(second.getState().selectedMapName, '');
  assert.equal(second.getState().selectedTileIndexInTileset, -1);
  assert.equal(second.getState().selectedTilesetName, '');
  assert.deepEqual(second.getState().rectCloneBrushTiles, []);
  assert.deepEqual(
    [second.input.translateX, second.input.translateY, second.input.scale],
    [0, 0, 1],
  );
  assert.equal(secondMap.selectedTileInd, -1);
  assert.deepEqual(secondMap.undoHistory, []);
  assert.equal(second.getCurrentAction(), null);
  assert.equal(first.getMapRevision('same-name'), 1);
  assert.equal(second.getMapRevision('same-name'), 0);
});

test('selection is unique inside one controller and isolated between controllers', () => {
  const first = new MapEditorController();
  const second = new MapEditorController();
  first.ensureMap('west');
  first.ensureMap('east');
  second.ensureMap('west').selectedTileInd = 99;

  setSoleSelectedTile(first, 'west', 3);
  assert.equal(first.getMapState('west')?.selectedTileInd, 3);
  assert.equal(first.getMapState('east')?.selectedTileInd, -1);

  setSoleSelectedTile(first, 'east', 4);
  assert.equal(first.getMapState('west')?.selectedTileInd, -1);
  assert.equal(first.getMapState('east')?.selectedTileInd, 4);
  assert.equal(second.getMapState('west')?.selectedTileInd, 99);

  clearAllSelectedTiles(first);
  assert.equal(first.getMapState('east')?.selectedTileInd, -1);
  assert.equal(second.getMapState('west')?.selectedTileInd, 99);
});

test('active strokes pin layer buffers until completion, then restore the bound', () => {
  const controller = new MapEditorController();
  const action = { type: 'DRAW' } as unknown as PaintAction;
  controller.setCurrentAction(action);

  for (let index = 0; index < 120; index++) {
    const name = `partition-${index}`;
    controller.ensureMap(name);
    controller.setLayerView(name, 0, [{ tileId: index } as unknown as never]);
  }

  assert.ok(controller.getLayerView('partition-0', 0));
  controller.setCurrentAction(null);
  assert.equal(controller.getLayerView('partition-0', 0), undefined);
  assert.ok(controller.getLayerView('partition-119', 0));
});

test('removing a map clears its state, undo order, and materialized layers', () => {
  const controller = new MapEditorController();
  controller.ensureMap('reused-name').selectedTileInd = 7;
  controller.getState().selectedMapName = 'reused-name';
  controller.getState().gridUndoOrder = ['other', 'reused-name'];
  controller.setLayerView('reused-name', 0, [
    { tileId: 42 } as unknown as never,
  ]);

  controller.removeMap('reused-name');

  assert.equal(controller.getMapState('reused-name'), undefined);
  assert.equal(controller.getLayerView('reused-name', 0), undefined);
  assert.equal(controller.getState().selectedMapName, '');
  assert.deepEqual(controller.getState().gridUndoOrder, ['other']);
  assert.equal(controller.ensureMap('reused-name').selectedTileInd, -1);
});

test('detaching aborts transient input and discards an uncommitted stroke', () => {
  const canvas = new FakeCanvas();
  const controller = new MapEditorController();
  controller.ensureMap('abort-map');
  controller.attach({
    canvas: canvas as unknown as HTMLCanvasElement,
    handlers: {},
  });
  controller.setLayerView('abort-map', 0, [{ tileId: 99 } as unknown as never]);
  controller.setCurrentAction({ type: 'DRAW' } as unknown as PaintAction);
  controller.input.isPainting = true;
  controller.input.isDragging = true;
  controller.getState().activePaintMapName = 'abort-map';

  controller.detach();

  assert.equal(controller.getCurrentAction(), null);
  assert.equal(controller.input.isPainting, false);
  assert.equal(controller.input.isDragging, false);
  assert.equal(controller.getState().activePaintMapName, '');
  assert.equal(controller.getLayerView('abort-map', 0), undefined);
});

test('shared keyboard target dispatches only to the focused controller canvas', () => {
  const keyboard = new TrackingTarget();
  const firstCanvas = new FakeCanvas();
  const secondCanvas = new FakeCanvas();
  let firstKeys = 0;
  let secondKeys = 0;
  const first = new MapEditorController({ keyboardTarget: keyboard });
  const second = new MapEditorController({ keyboardTarget: keyboard });
  first.attach({
    canvas: firstCanvas as unknown as HTMLCanvasElement,
    handlers: { keydown: () => firstKeys++ },
  });
  second.attach({
    canvas: secondCanvas as unknown as HTMLCanvasElement,
    handlers: { keydown: () => secondKeys++ },
  });

  firstCanvas.dispatchEvent(pointerEvent('pointerdown', 1));
  assert.equal(globalThis.document.activeElement, firstCanvas);
  assert.equal(keyboard.count('keydown'), 2);
  keyboard.dispatchEvent(new Event('keydown'));
  secondCanvas.dispatchEvent(pointerEvent('pointerdown', 2));
  assert.equal(globalThis.document.activeElement, secondCanvas);
  keyboard.dispatchEvent(new Event('keydown'));
  assert.deepEqual([firstKeys, secondKeys], [1, 1]);

  first.destroy();
  second.destroy();
});
