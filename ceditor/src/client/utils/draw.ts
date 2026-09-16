import {
  CarcerMapTileTemplate,
  TileMetadata,
} from '../types/assets';
import { Sprite } from './assetLoader';
import { disableCanvasSmoothing, getCachedDrawable } from './spriteUtils';

export interface DrawTextParams {
  font?: string;
  color?: string;
  size?: number;
  align?: 'left' | 'center' | 'right';
  strokeColor?: string;
  baseline?: 'top' | 'middle' | 'bottom';
}
const DEFAULT_TEXT_PARAMS = {
  font: 'monospace',
  color: '#fff',
  size: 14,
  align: 'center',
  strokeColor: 'black',
  baseline: 'middle',
};

let fm = 1;
export const setFm = (n: number) => {
  fm = n;
};
export const getFm = () => fm;

/**
 * Nudge pan so the map origin lands on integer canvas pixels.
 * Fractional pan/zoom offsets cause 1px gaps between tiles (black seams).
 */
export const snapPixelArtPanOffset = (
  panX: number,
  panY: number,
  scale: number,
  canvasW: number,
  canvasH: number,
  mapW: number,
  mapH: number,
  tileW: number,
  tileH: number
): { x: number; y: number } => {
  const mapPixelW = mapW * tileW;
  const mapPixelH = mapH * tileH;
  const originX = panX + (canvasW * scale) / 2 - (mapPixelW * scale) / 2;
  const originY = panY + (canvasH * scale) / 2 - (mapPixelH * scale) / 2;
  return {
    x: panX + Math.round(originX) - originX,
    y: panY + Math.round(originY) - originY,
  };
};

/**
 * Snap zoom so each tile occupies a whole number of canvas pixels.
 * Fractional tile sizes plus per-sprite rounding leave black seams when zoomed out.
 */
export const snapPixelArtScale = (scale: number, tileSize: number): number => {
  if (tileSize <= 0 || scale <= 0) {
    return scale;
  }
  const pixels = Math.max(1, Math.round(tileSize * scale));
  return pixels / tileSize;
};

/**
 * Destination rect for one map tile such that adjacent tiles share edges
 * (no gaps, no overlaps) even when tileSize * scale is not an integer.
 */
export const scaledTileRect = (
  tileX: number,
  tileY: number,
  tileWidth: number,
  tileHeight: number,
  scale: number,
): { x: number; y: number; w: number; h: number } => {
  const x = Math.round(tileX * tileWidth * scale);
  const y = Math.round(tileY * tileHeight * scale);
  const w = Math.max(1, Math.round((tileX + 1) * tileWidth * scale) - x);
  const h = Math.max(1, Math.round((tileY + 1) * tileHeight * scale) - y);
  return { x, y, w, h };
};

export const drawSprite = (
  sprite: Sprite,
  x: number,
  y: number,
  scale: number,
  ctx: CanvasRenderingContext2D,
  destW?: number,
  destH?: number,
  flipped = false,
) => {
  scale = scale || 1;

  const drawable = getCachedDrawable(sprite);
  if (!drawable) {
    // Sprite sheets are extracted asynchronously; a not-yet-ready drawable is
    // normal during load. Throwing here would abort the frame mid-transform.
    return;
  }

  disableCanvasSmoothing(ctx);
  const drawW = destW ?? Math.max(1, Math.round(sprite.width * scale));
  const drawH = destH ?? Math.max(1, Math.round(sprite.height * scale));
  const destX = Math.round(x);
  const destY = Math.round(y);
  if (flipped) {
    ctx.save();
    disableCanvasSmoothing(ctx);
    ctx.translate(destX + drawW, destY);
    ctx.scale(-1, 1);
    ctx.drawImage(drawable, 0, 0, drawW, drawH);
    ctx.restore();
    return;
  }
  ctx.drawImage(drawable, destX, destY, drawW, drawH);
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
  ctx.textBaseline = textParams.baseline || 'middle';
  if (strokeColor) {
    ctx.strokeStyle = strokeColor;
    ctx.lineWidth = 4;
    // Miter joins spike on sharp glyph corners (e.g. M); round keeps a clean outline.
    ctx.lineJoin = 'round';
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
