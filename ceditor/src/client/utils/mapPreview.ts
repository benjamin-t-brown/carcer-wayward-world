import { CarcerMapTemplate, CarcerMapTileTemplate } from '../types/assets';
import { Sprite } from './assetLoader';
import {
  disableCanvasSmoothing,
  getCachedDrawable,
  preparePixelArtCanvas,
} from './spriteUtils';
import { drawRect, getSpriteNameFromTile } from './draw';
import { materializeLayer } from './mapIndex';

export const MAP_PREVIEW_DISPLAY_SIZE = 100;

const MAP_PREVIEW_BACKGROUND = '#421';

export function getMapTilesForLevel(
  map: CarcerMapTemplate,
  level: string | number = '0'
): CarcerMapTileTemplate[] {
  return materializeLayer(map, parseInt(String(level), 10));
}

export function getMapPixelSize(map: CarcerMapTemplate): {
  width: number;
  height: number;
} {
  return {
    width: map.width * map.spriteWidth,
    height: map.height * map.spriteHeight,
  };
}

export function getMapPreviewDrawSize(
  map: CarcerMapTemplate,
  displaySize: number = MAP_PREVIEW_DISPLAY_SIZE
): { drawWidth: number; drawHeight: number; offsetX: number; offsetY: number } {
  const { width: mapW, height: mapH } = getMapPixelSize(map);
  const scale = Math.min(displaySize / mapW, displaySize / mapH);
  const drawWidth = Math.max(1, Math.floor(mapW * scale));
  const drawHeight = Math.max(1, Math.floor(mapH * scale));
  return {
    drawWidth,
    drawHeight,
    offsetX: Math.floor((displaySize - drawWidth) / 2),
    offsetY: Math.floor((displaySize - drawHeight) / 2),
  };
}

/** Renders base terrain tiles to an offscreen canvas at native map resolution. */
export function renderMapToCanvas(
  map: CarcerMapTemplate,
  spriteMap: Record<string, Sprite>
): HTMLCanvasElement | null {
  const { width: canvasWidth, height: canvasHeight } = getMapPixelSize(map);
  if (canvasWidth <= 0 || canvasHeight <= 0) {
    return null;
  }

  const canvas = document.createElement('canvas');
  const ctx = canvas.getContext('2d');
  if (!ctx) {
    return null;
  }

  canvas.width = canvasWidth;
  canvas.height = canvasHeight;
  disableCanvasSmoothing(ctx);

  drawRect(0, 0, canvasWidth, canvasHeight, MAP_PREVIEW_BACKGROUND, false, ctx);

  const tiles = getMapTilesForLevel(map);
  for (let y = 0; y < map.height; y++) {
    for (let x = 0; x < map.width; x++) {
      const tileIndex = y * map.width + x;
      const refTile = tiles[tileIndex];
      if (!refTile) {
        continue;
      }

      const spriteName = getSpriteNameFromTile(refTile);
      const sprite = spriteMap[spriteName];
      if (!sprite) {
        continue;
      }

      const drawable = getCachedDrawable(sprite);
      if (!drawable) {
        continue;
      }

      const tileX = x * map.spriteWidth;
      const tileY = y * map.spriteHeight;
      ctx.drawImage(
        drawable,
        tileX,
        tileY,
        map.spriteWidth,
        map.spriteHeight
      );
    }
  }

  return canvas;
}

export function drawMapPreviewToCanvas(
  targetCanvas: HTMLCanvasElement,
  map: CarcerMapTemplate,
  spriteMap: Record<string, Sprite>,
  displaySize: number = MAP_PREVIEW_DISPLAY_SIZE
): boolean {
  const source = renderMapToCanvas(map, spriteMap);
  if (!source) {
    return false;
  }

  const ctx = targetCanvas.getContext('2d');
  if (!ctx) {
    return false;
  }

  preparePixelArtCanvas(targetCanvas, ctx, displaySize, displaySize);
  ctx.fillStyle = MAP_PREVIEW_BACKGROUND;
  ctx.fillRect(0, 0, displaySize, displaySize);

  const { drawWidth, drawHeight, offsetX, offsetY } = getMapPreviewDrawSize(
    map,
    displaySize
  );
  disableCanvasSmoothing(ctx);
  ctx.drawImage(source, offsetX, offsetY, drawWidth, drawHeight);
  return true;
}
