import assert from 'node:assert/strict';
import test from 'node:test';

import {
  Viewport,
  boundedPixelRatio,
  clientToLogicalCanvasPoint,
  configureCanvasBackingStore,
  createCanvasMetrics,
  writeCanvasMetrics,
} from '../../src/apps/maps/canvas/index.js';

test('canvas metrics preserve logical size at DPR 1 and 2', () => {
  const metrics = createCanvasMetrics();

  assert.equal(writeCanvasMetrics(metrics, 320, 180, 1), true);
  assert.deepEqual(metrics, {
    logicalWidth: 320,
    logicalHeight: 180,
    backingWidth: 320,
    backingHeight: 180,
    pixelRatio: 1,
  });

  assert.equal(writeCanvasMetrics(metrics, 320, 180, 2), true);
  assert.deepEqual(metrics, {
    logicalWidth: 320,
    logicalHeight: 180,
    backingWidth: 640,
    backingHeight: 360,
    pixelRatio: 2,
  });
  assert.equal(writeCanvasMetrics(metrics, 320, 180, 2), false);
});

test('fractional CSS dimensions round only the backing store', () => {
  const metrics = createCanvasMetrics();
  writeCanvasMetrics(metrics, 100.25, 50.75, 1.5);

  assert.equal(metrics.logicalWidth, 100.25);
  assert.equal(metrics.logicalHeight, 50.75);
  assert.equal(metrics.backingWidth, 150);
  assert.equal(metrics.backingHeight, 76);
  assert.equal(metrics.pixelRatio, 1.5);
});

test('DPR and empty dimensions are safely bounded', () => {
  const metrics = createCanvasMetrics();
  writeCanvasMetrics(metrics, 0, -20, 20);

  assert.deepEqual(metrics, {
    logicalWidth: 0,
    logicalHeight: 0,
    backingWidth: 1,
    backingHeight: 1,
    pixelRatio: 4,
  });
  assert.equal(boundedPixelRatio(0), 1);
  assert.equal(boundedPixelRatio(Number.NaN), 1);
  assert.equal(boundedPixelRatio(3, 2), 2);
});

test('backing store resizes only when required and restores CSS transform', () => {
  const assignments: string[] = [];
  let width = 300;
  let height = 150;
  const canvas = {
    get width() {
      return width;
    },
    set width(value: number) {
      width = value;
      assignments.push(`width:${value}`);
    },
    get height() {
      return height;
    },
    set height(value: number) {
      height = value;
      assignments.push(`height:${value}`);
    },
  };
  const transforms: number[][] = [];
  const context = {
    setTransform(...values: number[]) {
      transforms.push(values);
    },
  };
  const metrics = createCanvasMetrics();
  writeCanvasMetrics(metrics, 300, 150, 2);

  assert.equal(configureCanvasBackingStore(canvas, context, metrics), true);
  assert.deepEqual(assignments, ['width:600', 'height:300']);
  assert.deepEqual(transforms.at(-1), [2, 0, 0, 2, 0, 0]);

  assert.equal(configureCanvasBackingStore(canvas, context, metrics), false);
  assert.deepEqual(assignments, ['width:600', 'height:300']);
  assert.equal(transforms.length, 2);
});

test('client coordinates stay logical at DPR 1 and DPR 2', () => {
  const point = { x: 0, y: 0 };
  const rect = { left: 125, top: 75, width: 400, height: 200 };
  const metrics = createCanvasMetrics();

  writeCanvasMetrics(metrics, rect.width, rect.height, 1);
  assert.deepEqual(clientToLogicalCanvasPoint(point, 325, 125, rect, metrics), {
    x: 200,
    y: 50,
  });

  writeCanvasMetrics(metrics, rect.width, rect.height, 2);
  assert.deepEqual(clientToLogicalCanvasPoint(point, 325, 125, rect, metrics), {
    x: 200,
    y: 50,
  });
});

test('pointer conversion accounts for CSS scaling and zero rectangles', () => {
  const point = { x: -1, y: -1 };
  const metrics = createCanvasMetrics();
  writeCanvasMetrics(metrics, 200, 100, 2);

  assert.deepEqual(
    clientToLogicalCanvasPoint(
      point,
      250,
      100,
      { left: 50, top: 50, width: 400, height: 200 },
      metrics,
    ),
    { x: 100, y: 25 },
  );
  assert.deepEqual(
    clientToLogicalCanvasPoint(
      point,
      250,
      100,
      { left: 50, top: 50, width: 0, height: 0 },
      metrics,
    ),
    { x: 0, y: 0 },
  );
});

test('DPR pointer conversion and viewport zoom preserve the world anchor', () => {
  const metrics = createCanvasMetrics();
  writeCanvasMetrics(metrics, 640.5, 360.25, 2);
  const point = clientToLogicalCanvasPoint(
    { x: 0, y: 0 },
    420.25,
    235.125,
    { left: 100, top: 55, width: 640.5, height: 360.25 },
    metrics,
  );
  const viewport = new Viewport({ x: -80, y: 25, scale: 1.25 });
  const worldX = viewport.screenToWorldX(point.x);
  const worldY = viewport.screenToWorldY(point.y);

  viewport.zoomAt(point.x, point.y, 1.75);
  assert.ok(Math.abs(viewport.worldToScreenX(worldX) - point.x) < 1e-10);
  assert.ok(Math.abs(viewport.worldToScreenY(worldY) - point.y) < 1e-10);
});
