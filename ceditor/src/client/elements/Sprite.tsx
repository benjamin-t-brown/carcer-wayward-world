import { useEffect, useRef } from 'react';
import { Sprite as SpriteType } from '../utils/assetLoader';
import { getCachedDrawable } from '../utils/spriteUtils';

interface SpriteProps {
  sprite?: SpriteType;
  scale?: number;
  className?: string;
  maxWidth?: number;
  onRender?: (ctx: CanvasRenderingContext2D) => void;
  renderDeps?: any[];
}

export function Sprite({
  sprite,
  scale = 1,
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

      if (!sprite) {
        // Fill with a purple rectangle and write "none"
        const width = 32 * scale;
        const height = 32 * scale;
        canvas.width = width;
        canvas.height = height;
        ctx.fillStyle = '#a259e6'; // purple color
        ctx.fillRect(0, 0, width, height);
        ctx.fillStyle = '#fff';
        ctx.font = `${12 * scale}px sans-serif`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText('none', width / 2, height / 2);
        return;
      }
      const drawable = getCachedDrawable(sprite);
      if (!drawable) {
        throw new Error('Drawable not found');
      }

      canvas.width = sprite.width * scale;
      canvas.height = sprite.height * scale;

      ctx.imageSmoothingEnabled = false;
      ctx.fillStyle = '#FFFFFF';
      ctx.fillRect(0, 0, sprite.width * scale, sprite.height * scale);
      ctx.drawImage(
        drawable,
        0,
        0,
        sprite.width * scale,
        sprite.height * scale
      );

      if (onRender) {
        onRender(ctx);
      }
    }

    loadSprite();
  }, [sprite, ...(renderDeps || [])]);

  return (
    <canvas
      ref={canvasRef}
      className={className}
      style={{
        display: 'inline-block',
        imageRendering: 'pixelated', // For pixel art
        maxWidth: maxWidth ? `${maxWidth}px` : undefined,
        // transform: `scale(${scale})`,
      }}
      draggable={false}
    />
  );
}
