import { useEffect, useRef } from 'react';
import type { Sprite as SpriteType } from '../utils/assetLoader';
import { getCachedDrawable, preparePixelArtCanvas } from '../utils/spriteUtils';

/** Largest integer scale that keeps the sprite within optional max dimensions. */
function clampIntegerScale(
  sprite: SpriteType,
  scale: number,
  maxWidth?: number,
  maxHeight?: number
): number {
  let s = Math.max(1, Math.floor(scale));
  if (maxWidth !== undefined) {
    s = Math.min(s, Math.max(1, Math.floor(maxWidth / sprite.width)));
  }
  if (maxHeight !== undefined) {
    s = Math.min(s, Math.max(1, Math.floor(maxHeight / sprite.height)));
  }
  return s;
}

/** Integer upscale when the sprite fits; otherwise shrink to fit the square. */
function scaleToDisplaySize(sprite: SpriteType, displaySize: number): number {
  const integerScale = Math.floor(
    Math.min(displaySize / sprite.width, displaySize / sprite.height)
  );
  if (integerScale >= 1) {
    return integerScale;
  }
  return Math.min(displaySize / sprite.width, displaySize / sprite.height);
}

interface SpriteProps {
  sprite?: SpriteType;
  scale?: number;
  /** Renders into a square canvas of this many CSS pixels (uniform fit, centered). */
  displaySize?: number;
  className?: string;
  maxWidth?: number;
  onRender?: (ctx: CanvasRenderingContext2D) => void;
  renderDeps?: unknown[];
}

export function Sprite({
  sprite,
  scale = 1,
  displaySize,
  className = '',
  maxWidth,
  onRender,
  renderDeps = [],
}: SpriteProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    async function loadSprite() {
      const canvas = canvasRef.current;
      if (!canvas) {
        throw new Error('Sprite ref Canvas not found');
      }

      const ctx = canvas.getContext('2d');
      if (!ctx) {
        throw new Error('Failed to get canvas context');
      }

      const placeholderSize = displaySize ?? 32 * scale;

      if (!sprite) {
        preparePixelArtCanvas(canvas, ctx, placeholderSize, placeholderSize);
        ctx.fillStyle = '#a259e6';
        ctx.fillRect(0, 0, placeholderSize, placeholderSize);
        ctx.fillStyle = '#fff';
        ctx.font = `${Math.max(10, placeholderSize * 0.375)}px sans-serif`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText('none', placeholderSize / 2, placeholderSize / 2);
        return;
      }

      const drawable = getCachedDrawable(sprite);
      if (!drawable) {
        throw new Error('Drawable not found');
      }

      if (displaySize !== undefined) {
        preparePixelArtCanvas(canvas, ctx, displaySize, displaySize);
        const fitScale = scaleToDisplaySize(sprite, displaySize);
        const drawW = Math.floor(sprite.width * fitScale);
        const drawH = Math.floor(sprite.height * fitScale);
        const offsetX = Math.floor((displaySize - drawW) / 2);
        const offsetY = Math.floor((displaySize - drawH) / 2);
        ctx.fillStyle = '#FFFFFF';
        ctx.fillRect(0, 0, displaySize, displaySize);
        ctx.drawImage(drawable, offsetX, offsetY, drawW, drawH);
      } else {
        const effectiveScale = clampIntegerScale(sprite, scale, maxWidth);
        const drawW = Math.floor(sprite.width * effectiveScale);
        const drawH = Math.floor(sprite.height * effectiveScale);
        preparePixelArtCanvas(canvas, ctx, drawW, drawH);
        ctx.fillStyle = '#FFFFFF';
        ctx.fillRect(0, 0, drawW, drawH);
        ctx.drawImage(drawable, 0, 0, drawW, drawH);
      }

      if (onRender) {
        onRender(ctx);
      }
    }

    loadSprite();
  }, [sprite, scale, displaySize, maxWidth, ...(renderDeps || [])]);

  return (
    <canvas
      ref={canvasRef}
      className={`sprite-render ${className}`.trim()}
      style={{ display: 'inline-block', verticalAlign: 'middle' }}
      draggable={false}
    />
  );
}
