import assert from 'node:assert/strict';
import test from 'node:test';

test('image and sprite requests share in-flight work and failed images can retry', async () => {
  const images: FakeImage[] = [];
  const canvases: FakeCanvas[] = [];

  class TestImage extends FakeImage {
    constructor() {
      super();
      images.push(this);
    }
  }

  Object.defineProperty(globalThis, 'Image', {
    configurable: true,
    value: TestImage,
  });
  Object.defineProperty(globalThis, 'document', {
    configurable: true,
    value: {
      createElement(tagName: string) {
        assert.equal(tagName, 'canvas');
        const canvas = new FakeCanvas();
        canvases.push(canvas);
        return canvas;
      },
    },
  });

  const { getDrawable, loadImage } =
    await import('../client/utils/spriteUtils');

  const firstImage = loadImage('/sheet.png');
  const secondImage = loadImage('/sheet.png');
  assert.equal(images.length, 1);
  images[0].succeed();
  assert.equal(await firstImage, await secondImage);

  const sprite = {
    name: 'sheet_0',
    pictureAlias: 'sheet',
    picturePath: 'sheet.png',
    index: 0,
    width: 16,
    height: 16,
  };
  const firstDrawable = getDrawable(sprite);
  const secondDrawable = getDrawable(sprite);
  assert.equal(images.length, 2);
  images[1].succeed();
  assert.equal(await firstDrawable, await secondDrawable);
  assert.equal(canvases.length, 1);
  assert.equal(canvases[0].drawCount, 1);

  const failedLoads = [loadImage('/missing.png'), loadImage('/missing.png')];
  assert.equal(images.length, 3);
  images[2].fail();
  const failures = await Promise.allSettled(failedLoads);
  assert.ok(failures.every(({ status }) => status === 'rejected'));

  const retry = loadImage('/missing.png');
  assert.equal(images.length, 4);
  images[3].succeed();
  assert.equal(await retry, images[3]);

  // The synchronous map renderer must hydrate its own sprite cache instead of
  // depending on a React <Sprite> preview to have rendered each tile first.
  const { drawSprite } = await import('../client/utils/draw');
  const mapSprite = {
    ...sprite,
    name: 'map-sheet_7',
    pictureAlias: 'map-sheet',
    picturePath: 'map-sheet.png',
    index: 7,
  };
  let renderedMapSprites = 0;
  const mapContext = {
    drawImage: () => {
      renderedMapSprites += 1;
    },
    imageSmoothingEnabled: true,
    imageSmoothingQuality: 'high',
  } as unknown as CanvasRenderingContext2D;

  drawSprite(mapSprite, 0, 0, 1, mapContext);
  drawSprite(mapSprite, 16, 0, 1, mapContext);
  assert.equal(renderedMapSprites, 0);
  assert.equal(images.length, 5, 'one sprite-sheet request is queued');

  const pendingMapDrawable = getDrawable(mapSprite);
  images[4].succeed();
  await pendingMapDrawable;

  drawSprite(mapSprite, 0, 0, 1, mapContext);
  assert.equal(renderedMapSprites, 1);
});

class FakeImage {
  width = 32;
  height = 16;
  onload: (() => void) | null = null;
  onerror: (() => void) | null = null;
  src = '';

  succeed(): void {
    this.onload?.();
  }

  fail(): void {
    this.onerror?.();
  }
}

class FakeCanvas {
  width = 0;
  height = 0;
  drawCount = 0;

  getContext(contextId: string): CanvasRenderingContext2D | null {
    assert.equal(contextId, '2d');
    return {
      drawImage: () => {
        this.drawCount += 1;
      },
      imageSmoothingEnabled: true,
      imageSmoothingQuality: 'high',
    } as unknown as CanvasRenderingContext2D;
  }
}
