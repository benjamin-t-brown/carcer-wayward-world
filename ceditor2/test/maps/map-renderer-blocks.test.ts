import assert from 'node:assert/strict';
import test from 'node:test';

import type { SpriteDefinition } from '../../src/core/media/index.js';
import {
  ImageCache,
  MapRenderer,
  Viewport,
  type RenderMapBlock,
  type RenderMapDocument,
} from '../../src/apps/maps/canvas/index.js';

test('renderer composes an ordered iterable at independent world origins', async () => {
  const source = {} as CanvasImageSource;
  const images = new ImageCache(async () => ({
    source,
    width: 20,
    height: 10,
  }));
  await images.load('sprites/loaded.png');

  const visitedLayers: number[] = [];
  const drawnZero = documentFor(sprite('sprites/loaded.png', 0), visitedLayers);
  const drawnOne = documentFor(sprite('sprites/loaded.png', 1), visitedLayers);
  const blank = documentFor(null, visitedLayers);
  const unresolved = documentFor(
    { ...sprite('sprites/loaded.png', 0), width: 0 },
    visitedLayers,
  );
  const pending = documentFor(sprite('sprites/pending.png', 0), visitedLayers);
  const offscreen = documentFor(sprite('sprites/loaded.png', 0), visitedLayers);
  const blocks = function* (): Iterable<RenderMapBlock> {
    yield block(drawnZero, 7, 0);
    yield block(drawnOne, 8, 10);
    yield block(blank, 9, 20);
    yield block(unresolved, 10, 30);
    yield block(pending, 11, 40);
    yield block(offscreen, 12, 100);
  };

  const drawCalls: unknown[][] = [];
  const context = fakeContext(200, 40, drawCalls);
  const renderer = new MapRenderer(images);
  const viewport = new Viewport();

  // The backing store models DPR 4; composition and culling use logical size.
  renderer.beginFrame(context, 50, 10);
  assert.equal(renderer.drawMaps(context, viewport, blocks()), 5);

  assert.deepEqual(visitedLayers, [7, 8, 9, 10, 11]);
  assert.equal(drawCalls.length, 2);
  assert.deepEqual(drawCalls[0]?.slice(1), [0, 0, 10, 10, 0, 0, 10, 10]);
  assert.deepEqual(drawCalls[1]?.slice(1), [10, 0, 10, 10, 10, 0, 10, 10]);
  assert.deepEqual(renderer.stats, {
    mapBlocks: 5,
    visitedTiles: 5,
    drawnTiles: 2,
    blankTiles: 1,
    unresolvedSprites: 1,
    pendingImages: 1,
  });
});

test('composition culls against logical dimensions instead of the DPR backing store', async () => {
  const source = {} as CanvasImageSource;
  const images = new ImageCache(async () => ({
    source,
    width: 10,
    height: 10,
  }));
  await images.load('sprites/loaded.png');
  const visible = documentFor(sprite('sprites/loaded.png', 0));
  const beyondLogicalEdge = documentFor(sprite('sprites/loaded.png', 0));
  const blocks: RenderMapBlock[] = [
    block(visible, 0, 10),
    block(beyondLogicalEdge, 0, 20),
  ];
  const drawCalls: unknown[][] = [];
  const context = fakeContext(80, 40, drawCalls);
  const renderer = new MapRenderer(images);

  renderer.beginFrame(context, 20, 10);
  assert.equal(renderer.drawMaps(context, new Viewport(), blocks), 1);
  assert.equal(drawCalls.length, 1);
  assert.equal(renderer.stats.mapBlocks, 1);
  assert.equal(renderer.stats.visitedTiles, 1);
});

test('fractional block backgrounds share rounded edges and restore opacity', async () => {
  const fillCalls: number[][] = [];
  const imageAlphas: number[] = [];
  const alphaStack: number[] = [];
  let currentAlpha = 1;
  const context = {
    canvas: { width: 100, height: 100 },
    get globalAlpha() {
      return currentAlpha;
    },
    set globalAlpha(value: number) {
      currentAlpha = value;
    },
    imageSmoothingEnabled: true,
    clearRect() {},
    fillRect(...values: number[]) {
      fillCalls.push(values);
    },
    save() {
      alphaStack.push(currentAlpha);
    },
    restore() {
      currentAlpha = alphaStack.pop() ?? 1;
    },
    drawImage() {
      imageAlphas.push(currentAlpha);
    },
  } as unknown as CanvasRenderingContext2D;
  const images = new ImageCache(async () => ({
    source: {} as CanvasImageSource,
    width: 10,
    height: 10,
  }));
  await images.load('sprites/loaded.png');
  const document = documentFor(sprite('sprites/loaded.png', 0));
  const renderer = new MapRenderer(images);
  const viewport = new Viewport({ x: 0.6, y: 0.6, scale: 1.25 });

  renderer.beginFrame(context, 100, 100);
  renderer.drawMaps(context, viewport, [
    {
      document,
      layer: 0,
      originX: 0,
      originY: 0,
      opacity: 0.5,
      background: '#000',
    },
    {
      document,
      layer: 0,
      originX: 10,
      originY: 0,
      background: '#000',
    },
  ]);

  assert.deepEqual(fillCalls.slice(-2), [
    [1, 1, 12, 12],
    [13, 1, 13, 12],
  ]);
  const leftBackground = fillCalls.at(-2);
  const rightBackground = fillCalls.at(-1);
  assert.ok(leftBackground);
  assert.ok(rightBackground);
  assert.equal(leftBackground[0]! + leftBackground[2]!, 13);
  assert.equal(rightBackground[0], 13);
  assert.deepEqual(imageAlphas, [0.5, 1]);
  assert.equal(context.globalAlpha, 1);
});

test('zero logical canvas size never falls back to DPR backing dimensions', () => {
  const context = fakeContext(1, 1, []);
  const renderer = new MapRenderer();
  renderer.beginFrame(context, 0, 0);

  assert.equal(
    renderer.drawMaps(context, new Viewport(), [
      block(documentFor(null), 0, 0),
    ]),
    0,
  );
  assert.equal(renderer.stats.visitedTiles, 0);
});

function block(
  document: RenderMapDocument,
  layer: number,
  originX: number,
): RenderMapBlock {
  return { document, layer, originX, originY: 0 };
}

function documentFor(
  result: SpriteDefinition | null | undefined,
  visitedLayers: number[] = [],
): RenderMapDocument {
  return {
    width: 1,
    height: 1,
    tileWidth: 10,
    tileHeight: 10,
    spriteAt(layer) {
      visitedLayers.push(layer);
      return result;
    },
  };
}

function sprite(path: string, index: number): SpriteDefinition {
  return {
    name: `sprite_${index}`,
    pictureAlias: 'test',
    picturePath: path,
    index,
    width: 10,
    height: 10,
  };
}

function fakeContext(
  width: number,
  height: number,
  drawCalls: unknown[][],
): CanvasRenderingContext2D {
  return {
    canvas: { width, height },
    globalAlpha: 1,
    imageSmoothingEnabled: true,
    clearRect() {},
    fillRect() {},
    save() {},
    restore() {},
    drawImage(...args: unknown[]) {
      drawCalls.push(args);
    },
  } as unknown as CanvasRenderingContext2D;
}
