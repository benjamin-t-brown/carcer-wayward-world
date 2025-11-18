// import { Sprite } from '../../../assetLoader';

import { Sprite } from './assetLoader';

const imageLoaders: Record<string, (() => void)[]> = {};
const imageCache: Record<string, HTMLImageElement> = {};
// const spriteCanvasCache: Record<string, HTMLCanvasElement> = {};
const spriteImageCache: Record<string, HTMLImageElement> = {};

(window as any).spriteImageCache = spriteImageCache;

export async function loadImage(url: string): Promise<HTMLImageElement> {
  if (imageCache[url]) {
    return imageCache[url];
  }
  if (imageLoaders[url]) {
    return new Promise((resolve) => {
      imageLoaders[url].push(() => {
        resolve(imageCache[url]);
      });
    });
  }
  const img = new Image();

  return new Promise((resolve, reject) => {
    imageLoaders[url] = imageLoaders[url] || [];
    imageLoaders[url].push(() => {
      resolve(imageCache[url]);
    });
    img.onload = () => {
      console.log('image loaded', url);
      imageCache[url] = img;
      imageLoaders[url]?.forEach((loader) => loader());
    };
    img.onerror = () => {
      reject(new Error(`Failed to load image: ${url}`));
    };
    console.log('loading image', url);
    img.src = url;
  });
}

function getSpriteSheetUrl(picturePath: string): string {
  return `/api/${picturePath}`;
}

const outerCanvas = document.createElement('canvas');
outerCanvas.width = 1024;
outerCanvas.height = 1024;
const outerCtx = outerCanvas.getContext('2d');
if (!outerCtx || !outerCanvas) {
  throw new Error('Failed to get context');
}

export async function getDrawable(sprite: Sprite) {
  const cacheKey = `${sprite.name}`;
  if (spriteImageCache[cacheKey]) {
    return spriteImageCache[cacheKey];
  }
  const imageUrl = getSpriteSheetUrl(sprite.picturePath);
  const image = await loadImage(imageUrl);
  const numSpritesWide = image.width / sprite.width;
  const xOffset = (sprite.index % numSpritesWide) * sprite.width;
  const yOffset = Math.floor(sprite.index / numSpritesWide) * sprite.height;
  if (!outerCtx) {
    throw new Error('Failed to get context');
  }
  outerCanvas.width = sprite.width;
  outerCanvas.height = sprite.height;
  outerCtx.clearRect(0, 0, outerCanvas.width, outerCanvas.height);
  outerCtx.drawImage(
    image,
    xOffset,
    yOffset,
    sprite.width,
    sprite.height,
    0,
    0,
    sprite.width,
    sprite.height
  );
  // spriteCanvasCache[cacheKey] = canvas;
  // Create an Image and set its src to the canvas data URL
  const img = new window.Image();
  img.src = outerCanvas.toDataURL();
  spriteImageCache[cacheKey] = img;
  return img;
}

export function getCachedDrawable(sprite: Sprite) {
  const cacheKey = `${sprite.name}`;
  if (spriteImageCache[cacheKey]) {
    return spriteImageCache[cacheKey];
  }
}
