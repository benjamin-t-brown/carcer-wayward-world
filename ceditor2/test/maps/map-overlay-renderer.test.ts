import assert from 'node:assert/strict';
import test from 'node:test';

import { MapOverlayRenderer } from '../../src/apps/maps/canvas/MapOverlayRenderer.js';
import { Viewport } from '../../src/apps/maps/canvas/Viewport.js';
import { MapDocument } from '../../src/core/domain/maps/index.js';

test('overlay renderer culls sparse placements by layer and viewport', () => {
  const calls: string[] = [];
  const context = {
    fillStyle: '',
    font: '',
    textBaseline: '',
    save: () => calls.push('save'),
    restore: () => calls.push('restore'),
    fillRect: (x: number, y: number) => calls.push(`rect:${x},${y}`),
    fillText: (text: string, x: number, y: number) =>
      calls.push(`text:${text}:${x},${y}`),
  } as unknown as CanvasRenderingContext2D;
  const map = MapDocument.from({
    name: 'map',
    width: 3,
    height: 1,
    spriteWidth: 20,
    spriteHeight: 20,
    tilesets: [''],
    layers: [0, 1],
    tiles: {
      '0': [0, 0, 0, 0, 0, 0],
      '1': [0, 0, 0, 0, 0, 0],
    },
    characters: [
      { l: 0, i: 0, name: 'Ada' },
      { l: 1, i: 1, name: 'Other layer' },
    ],
    eventTriggers: [{ l: 0, i: 2, eventId: 'offscreen' }],
  });
  const drawn = new MapOverlayRenderer().draw(
    context,
    new Viewport(),
    [{ document: map, originX: 0, originY: 0 }],
    { layer: 0, canvasWidth: 39, canvasHeight: 20, showLabels: true },
  );

  assert.equal(drawn, 1);
  assert.deepEqual(calls, [
    'save',
    'rect:2,2',
    'text:C:4,3',
    'text:Ada:2,8',
    'restore',
  ]);
});

test('overlay block origins align metadata across map partitions', () => {
  const rectangles: number[] = [];
  const context = {
    fillStyle: '',
    font: '',
    textBaseline: '',
    save() {},
    restore() {},
    fillRect(x: number) {
      rectangles.push(x);
    },
    fillText() {},
  } as unknown as CanvasRenderingContext2D;
  const document = MapDocument.from({
    name: 'east',
    width: 1,
    height: 1,
    spriteWidth: 28,
    spriteHeight: 32,
    tilesets: [''],
    layers: [0],
    tiles: { '0': [0, 0] },
    markers: [{ name: 'door' }],
  });

  const drawn = new MapOverlayRenderer().draw(
    context,
    new Viewport({ x: 5, y: 0, scale: 2 }),
    [{ document, originX: 56, originY: 0 }],
    { layer: 0, canvasWidth: 200, canvasHeight: 100 },
  );
  assert.equal(drawn, 1);
  assert.deepEqual(rectangles, [119]);
});
