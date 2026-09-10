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
