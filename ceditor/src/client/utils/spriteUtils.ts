import { Sprite } from './assetLoader';

const imageCache: Record<string, HTMLImageElement> = {};
const imagePromiseCache: Partial<Record<string, Promise<HTMLImageElement>>> =
  {};
const spriteCanvasCache: Record<string, HTMLCanvasElement> = {};
const spritePromiseCache: Partial<Record<string, Promise<HTMLCanvasElement>>> =
  {};

export async function loadImage(url: string): Promise<HTMLImageElement> {
  if (imageCache[url]) {
    return imageCache[url];
  }
  if (imagePromiseCache[url]) {
    return imagePromiseCache[url];
  }

  const img = new Image();
  const loadPromise = new Promise<HTMLImageElement>((resolve, reject) => {
    img.onload = () => {
      imageCache[url] = img;
      delete imagePromiseCache[url];
      resolve(img);
    };
    img.onerror = () => {
      delete imagePromiseCache[url];
      reject(new Error(`Failed to load image: ${url}`));
    };
    img.src = url;
  });
  imagePromiseCache[url] = loadPromise;
  return loadPromise;
}

function getSpriteSheetUrl(picturePath: string): string {
  return `/api/${picturePath}`;
}

/** Nearest-neighbor scaling when drawing pixel art. */
export function disableCanvasSmoothing(ctx: CanvasRenderingContext2D): void {
  ctx.imageSmoothingEnabled = false;
  ctx.imageSmoothingQuality = 'low';
}

/**
 * Size the canvas backing store for HiDPI while drawing in logical pixel units.
 */
export function preparePixelArtCanvas(
  canvas: HTMLCanvasElement,
  ctx: CanvasRenderingContext2D,
  logicalWidth: number,
  logicalHeight: number,
): void {
  const dpr = window.devicePixelRatio || 1;
  canvas.width = Math.max(1, Math.floor(logicalWidth * dpr));
  canvas.height = Math.max(1, Math.floor(logicalHeight * dpr));
  canvas.style.width = `${logicalWidth}px`;
  canvas.style.height = `${logicalHeight}px`;
  ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  disableCanvasSmoothing(ctx);
}

const SPRITE_CACHE_VERSION = 'v3';

export async function getDrawable(sprite: Sprite): Promise<HTMLCanvasElement> {
  const cacheKey = `${sprite.name}@${SPRITE_CACHE_VERSION}`;
  if (spriteCanvasCache[cacheKey]) {
    return spriteCanvasCache[cacheKey];
  }
  if (spritePromiseCache[cacheKey]) {
    return spritePromiseCache[cacheKey];
  }

  const drawablePromise = (async () => {
    const imageUrl = getSpriteSheetUrl(sprite.picturePath);
    const image = await loadImage(imageUrl);
    const numSpritesWide = image.width / sprite.width;
    const xOffset = (sprite.index % numSpritesWide) * sprite.width;
    const yOffset = Math.floor(sprite.index / numSpritesWide) * sprite.height;

    const canvas = document.createElement('canvas');
    canvas.width = sprite.width;
    canvas.height = sprite.height;
    const ctx = canvas.getContext('2d');
    if (!ctx) {
      throw new Error('Failed to get sprite extract context');
    }
    disableCanvasSmoothing(ctx);
    ctx.drawImage(
      image,
      xOffset,
      yOffset,
      sprite.width,
      sprite.height,
      0,
      0,
      sprite.width,
      sprite.height,
    );
    spriteCanvasCache[cacheKey] = canvas;
    return canvas;
  })();

  spritePromiseCache[cacheKey] = drawablePromise;
  try {
    return await drawablePromise;
  } finally {
    delete spritePromiseCache[cacheKey];
  }
}

export function getCachedDrawable(
  sprite: Sprite,
): HTMLCanvasElement | undefined {
  const cacheKey = `${sprite.name}@${SPRITE_CACHE_VERSION}`;
  const cached = spriteCanvasCache[cacheKey];
  if (cached) {
    return cached;
  }

  // Canvas render loops must stay synchronous, but cannot rely on a React
  // <Sprite> preview to have warmed the cache. Queue one extraction per unique
  // sprite; a subsequent frame will receive the cached drawable.
  if (!spritePromiseCache[cacheKey]) {
    void getDrawable(sprite).catch((error: unknown) => {
      console.error(`Failed to load sprite drawable: ${sprite.name}`, error);
    });
  }

  return undefined;
}

export function getCachedImage(imagePath: string) {
  imagePath = '/api/' + imagePath;
  if (imageCache[imagePath]) {
    return imageCache[imagePath];
  }
}
