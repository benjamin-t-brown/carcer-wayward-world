// import { Sprite } from '../../../assetLoader';

import { Sprite } from './assetLoader';

const imageLoaders: Record<string, (() => void)[]> = {};
const imageCache: Record<string, HTMLImageElement> = {};
const spriteCanvasCache: Record<string, HTMLCanvasElement> = {};

async function loadImage(url: string): Promise<HTMLImageElement> {
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

export async function getDrawable(sprite: Sprite) {
  const cacheKey = `${sprite.name}`;
  if (spriteCanvasCache[cacheKey]) {
    return spriteCanvasCache[cacheKey];
  }
  const imageUrl = getSpriteSheetUrl(sprite.picturePath);
  const image = await loadImage(imageUrl);
  const canvas = document.createElement('canvas');
  canvas.width = sprite.width;
  canvas.height = sprite.height;
  const ctx = canvas.getContext('2d');
  if (!ctx) {
    throw new Error('Failed to get context');
  }
  const numSpritesWide = image.width / sprite.width;
  // const numSpritesHigh = image.height / sprite.height;
  const xOffset = (sprite.index % numSpritesWide) * sprite.width;
  const yOffset = Math.floor(sprite.index / numSpritesWide) * sprite.height;
  // console.log(
  //   'draw image',
  //   sprite.name,
  //   xOffset,
  //   yOffset,
  //   sprite.width,
  //   sprite.height
  // );
  ctx.drawImage(
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
  spriteCanvasCache[cacheKey] = canvas;
  return canvas;
}
