import assert from 'node:assert/strict';
import test from 'node:test';

import type { SpriteDefinition } from '../../src/core/media/index.js';
import {
  ImageCache,
  MapRenderer,
  Viewport,
  gameAssetUrl,
  normalizeWheelDelta,
  wheelZoomFactor,
  type RenderMapDocument,
  type TileBounds,
} from '../../src/apps/maps/canvas/index.js';

test('viewport converts between world and screen coordinates', () => {
  const viewport = new Viewport({ x: 12, y: -8, scale: 2 });
  const point = { x: 0, y: 0 };

  assert.deepEqual(viewport.worldToScreen(7, 9, point), { x: 26, y: 10 });
  assert.deepEqual(viewport.screenToWorld(26, 10, point), { x: 7, y: 9 });
});

test('pointer-anchored zoom clamps scale and preserves the anchor', () => {
  const viewport = new Viewport({ x: 20, y: 30, scale: 1 });
  const worldX = viewport.screenToWorldX(150);
  const worldY = viewport.screenToWorldY(90);

  assert.equal(viewport.zoomAt(150, 90, 2), 2);
  assert.equal(viewport.worldToScreenX(worldX), 150);
  assert.equal(viewport.worldToScreenY(worldY), 90);
  assert.equal(viewport.zoomAt(150, 90, 100), 10);
  assert.equal(viewport.zoomAt(150, 90, 0.0001), 0.5);
});

test('pan and center use logical canvas pixels', () => {
  const viewport = new Viewport({ scale: 2 });
  viewport.centerOn(50, 25, 400, 200);
  assert.deepEqual(viewport.worldToScreen(50, 25, { x: 0, y: 0 }), {
    x: 200,
    y: 100,
  });
  viewport.panBy(15, -10);
  assert.deepEqual(viewport.worldToScreen(50, 25, { x: 0, y: 0 }), {
    x: 215,
    y: 90,
  });
});

test('visible bounds are inclusive, clipped, reusable, and block-aware', () => {
  const viewport = new Viewport({ x: -100, y: -50, scale: 1 });
  const bounds: TileBounds = { minX: 99, maxX: 99, minY: 99, maxY: 99 };
  const visible = viewport.writeVisibleTileBounds(bounds, {
    originX: 0,
    originY: 0,
    mapWidth: 100,
    mapHeight: 100,
    tileWidth: 10,
    tileHeight: 10,
    canvasWidth: 25,
    canvasHeight: 25,
  });

  assert.equal(visible, true);
  assert.deepEqual(bounds, { minX: 10, maxX: 12, minY: 5, maxY: 7 });

  assert.equal(
    viewport.writeVisibleTileBounds(bounds, {
      originX: 1000,
      originY: 1000,
      mapWidth: 5,
      mapHeight: 5,
      tileWidth: 10,
      tileHeight: 10,
      canvasWidth: 25,
      canvasHeight: 25,
    }),
    false,
  );
  assert.deepEqual(bounds, { minX: 0, maxX: -1, minY: 0, maxY: -1 });
});

test('wheel helpers normalize modes and produce reciprocal zoom direction', () => {
  assert.equal(normalizeWheelDelta(2, 0, 800), 2);
  assert.equal(normalizeWheelDelta(2, 1, 800), 32);
  assert.equal(normalizeWheelDelta(2, 2, 800), 1600);
  assert.ok(wheelZoomFactor(-100) > 1);
  assert.ok(wheelZoomFactor(100) < 1);
});

test('image cache coalesces loads and exposes completed resources', async () => {
  let loads = 0;
  const source = {} as CanvasImageSource;
  const cache = new ImageCache(async () => {
    loads += 1;
    return { source, width: 64, height: 32 };
  });

  const first = cache.load('sprites/test.png');
  const second = cache.load('sprites/test.png');
  assert.equal(first, second);
  assert.deepEqual(await first, { source, width: 64, height: 32 });
  assert.equal(cache.get('sprites/test.png')?.source, source);
  assert.equal(loads, 1);
  assert.equal(
    gameAssetUrl('./sprites/test.png'),
    '/game-assets/sprites/test.png',
  );
  assert.equal(
    gameAssetUrl('assets/img/tiles.png'),
    '/game-assets/img/tiles.png',
  );
});

test('renderer visits and draws only visible cells using sheet metadata', async () => {
  const source = {} as CanvasImageSource;
  const images = new ImageCache(async () => ({
    source,
    width: 40,
    height: 20,
  }));
  await images.load('sprites/test.png');

  const sprite: SpriteDefinition = {
    name: 'test_1',
    pictureAlias: 'test',
    picturePath: 'sprites/test.png',
    index: 1,
    width: 10,
    height: 10,
  };
  const visited: number[] = [];
  const document: RenderMapDocument = {
    width: 100,
    height: 100,
    tileWidth: 10,
    tileHeight: 10,
    spriteAt(_layer, tileIndex) {
      visited.push(tileIndex);
      return sprite;
    },
  };
  const drawCalls: unknown[][] = [];
  // Backing dimensions model DPR 2; beginFrame still receives 25x25 logical CSS pixels.
  const context = fakeContext(50, 50, drawCalls);
  const renderer = new MapRenderer(images);
  const viewport = new Viewport({ x: -100, y: -50, scale: 1 });

  renderer.beginFrame(context, 25, 25);
  assert.equal(
    renderer.drawMap(context, { document, layer: 0, viewport }),
    true,
  );

  assert.deepEqual(visited, [510, 511, 512, 610, 611, 612, 710, 711, 712]);
  assert.equal(drawCalls.length, 9);
  assert.equal(renderer.stats.visitedTiles, 9);
  assert.equal(renderer.stats.drawnTiles, 9);
  assert.equal(renderer.stats.pendingImages, 0);
  assert.deepEqual(drawCalls[0]?.slice(1, 5), [10, 0, 10, 10]);
});

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
