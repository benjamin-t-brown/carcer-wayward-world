import { useState, useEffect, useRef } from 'react';
import { Sprite as SpriteType } from '../utils/assetLoader';
import { getDrawable } from '../utils/spriteUtils';

interface SpriteProps {
  sprite: SpriteType;
  scale?: number;
  className?: string;
}

export function Sprite({ sprite, scale = 1, className = '' }: SpriteProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    async function loadSprite() {
      const drawable = await getDrawable(sprite);
      // console.log('sourceCanvas', drawable);
      const canvas = canvasRef.current;
      if (!canvas) {
        throw new Error('Sprite ref Canvas not found');
      }

      const ctx = canvas.getContext('2d');
      if (!ctx) {
        throw new Error('Failed to get canvas context');
      }

      canvas.width = sprite.width * scale;
      canvas.height = sprite.height * scale;

      ctx.fillStyle = '#FFFFFF';
      ctx.fillRect(0, 0, sprite.width * scale, sprite.height * scale);
      ctx.imageSmoothingEnabled = false;
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
        // transform: `scale(${scale})`,
      }}
      draggable={false}
    />
  );
}
