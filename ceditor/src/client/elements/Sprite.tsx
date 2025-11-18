import { useEffect, useRef } from 'react';
import { Sprite as SpriteType } from '../utils/assetLoader';
import { getCachedDrawable } from '../utils/spriteUtils';

interface SpriteProps {
  sprite: SpriteType;
  scale?: number;
  className?: string;
  maxWidth?: number;
}

export function Sprite({ sprite, scale = 1, className = '', maxWidth }: SpriteProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    async function loadSprite() {
      const drawable = getCachedDrawable(sprite);
      if (!drawable) {
        throw new Error('Drawable not found');
      }
      const canvas = canvasRef.current;
      if (!canvas) {
        throw new Error('Sprite ref Canvas not found');
      }
      
      const ctx = canvas.getContext('2d');
      if (!ctx) {
        throw new Error('Failed to get canvas context');
      }
      // console.log('sourceCanvas', sprite.name, drawable);

      canvas.width = sprite.width * scale;
      canvas.height = sprite.height * scale;

      ctx.imageSmoothingEnabled = false;
      ctx.fillStyle = '#FFFFFF';
      ctx.fillRect(0, 0, sprite.width * scale, sprite.height * scale);
      ctx.drawImage(drawable, 0, 0, sprite.width * scale, sprite.height * scale);
    }

    loadSprite();
  }, [sprite]);

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
