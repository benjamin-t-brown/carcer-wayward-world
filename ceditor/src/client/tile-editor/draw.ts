import {
  CarcerMapTileTemplate,
  TileMetadata,
} from '../types/assets';
import { Sprite } from '../utils/assetLoader';
import { getCachedDrawable } from '../utils/spriteUtils';

export interface DrawTextParams {
  font?: string;
  color?: string;
  size?: number;
  align?: 'left' | 'center' | 'right';
  strokeColor?: string;
}
const DEFAULT_TEXT_PARAMS = {
  font: 'monospace',
  color: '#fff',
  size: 14,
  align: 'center',
  strokeColor: 'black',
};

const globalScale = 4;

let fm = 1;
export const setFm = (n: number) => {
  fm = n;
};
export const getFm = () => fm;

const spriteCacheColor: Record<string, Sprite> = {};

// export const setupDrawing = () => {
//   const ctx = getCtx();
//   ctx.save();
//   ctx.translate(0, 0);
//   ctx.scale(globalScale, globalScale);
// };

// export const finishDrawing = () => {
//   getCtx().restore();
// };
// const getSpriteFromCache = (spriteName: string, color: string) => {
//   const key = `${spriteName}_${color}`;
//   if (!spriteCacheColor[key]) {
//     const sprite = getSprite(spriteName);
//     if (!sprite) {
//       throw new Error(`No sprite: "${spriteName}"`);
//     }
//     const [canvas, ctx] = createCanvas(sprite[3], sprite[4]);

//     const [, , , sprW, sprH] = sprite;
//     ctx.globalCompositeOperation = 'multiply';
//     ctx.fillStyle = color;
//     ctx.fillRect(0, 0, sprW, sprH);
//     ctx.globalCompositeOperation = 'destination-atop';

//     drawSpriteObj(sprite, 0, 0, 1, ctx);
//     spriteCacheColor[key] = [canvas, 0, 0, canvas.width, canvas.height];
//   }
//   return spriteCacheColor[key];
// };

// const drawSpriteObj = (
//   sprite: Sprite,
//   x: number,
//   y: number,
//   scale?: number,
//   ctx?: CanvasRenderingContext2D
// ) => {
//   scale = scale || 1;
//   ctx = ctx || getCtx();
//   const [image, sprX, sprY, sprW, sprH] = sprite;

//   ctx.drawImage(
//     image,
//     sprX,
//     sprY,
//     sprW,
//     sprH,
//     x,
//     y,
//     sprW * scale,
//     sprH * scale
//   );
//   ctx.restore();
// };

export const drawSprite = (
  sprite: Sprite,
  x: number,
  y: number,
  scale: number,
  ctx: CanvasRenderingContext2D
) => {
  scale = scale || 1;

  const drawable = getCachedDrawable(sprite);
  if (!drawable) {
    throw new Error('Drawable not found');
  }

  ctx.drawImage(drawable, x, y, sprite.width * scale, sprite.height * scale);
};

export const drawRect = (
  x: number,
  y: number,
  w: number,
  h: number,
  color: string,
  stroke: boolean,
  ctx: CanvasRenderingContext2D
) => {
  ctx[stroke ? 'strokeStyle' : 'fillStyle'] = color;
  ctx[stroke ? 'strokeRect' : 'fillRect'](x, y, w, h);
};

export const drawLine = (
  x1: number,
  y1: number,
  x2: number,
  y2: number,
  color: string,
  width: number,
  ctx: CanvasRenderingContext2D
) => {
  ctx.strokeStyle = color;
  ctx.lineWidth = width;
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.stroke();
};

export const drawText = (
  text: string,
  x: number,
  y: number,
  textParams: DrawTextParams,
  ctx: CanvasRenderingContext2D
) => {
  const { font, size, color, align, strokeColor } = {
    ...DEFAULT_TEXT_PARAMS,
    ...(textParams || {}),
  };
  ctx.font = `${size}px ${font}`;
  ctx.textAlign = align as CanvasTextAlign;
  ctx.textBaseline = 'middle';
  if (strokeColor) {
    ctx.strokeStyle = strokeColor;
    ctx.lineWidth = 4;
    ctx.strokeText(text, x, y);
  }

  ctx.fillStyle = color;
  ctx.fillText(text, x, y);
};

export const getSpriteNameFromTile = (tile: CarcerMapTileTemplate) => {
  return tile.tilesetName + '_' + tile.tileId;
};

export const getSpriteNameFromTileMetadata = (
  spriteSheetName: string,
  meta: TileMetadata
) => {
  return spriteSheetName + '_' + meta.id;
};

// const spriteToCanvas = (sprite: Sprite): HTMLCanvasElement => {
//   const [, , , spriteWidth, spriteHeight] = sprite;
//   const [canvas, ctx] = createCanvas(spriteWidth, spriteHeight);
//   drawSprite(sprite, 0, 0, 1, '', ctx);
//   return canvas;
// };

// type ImageDef = [string, string, number, number];
// export const loadImagesAndSprites = async (images: ImageDef[]) => {
//   getCanvas();
//   const imageMap = {};
//   const spriteMap = {};
//   await Promise.all(
//     images.map(([imageName, imagePath, spriteWidth, spriteHeight], i) => {
//       return new Promise<void>((resolve) => {
//         const img = new Image();
//         img.onload = () => {
//           imageMap[imageName] = img;
//           loadSpritesheet(spriteMap, img, imageName, spriteWidth, spriteHeight);
//           resolve();
//         };
//         img.src = imagePath;
//       });
//     })
//   );

//   model_images = imageMap;
//   model_sprites = spriteMap;

//   console.log(model_sprites);
// };
