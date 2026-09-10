import { useState, useEffect, useRef } from 'react';
import {
  Animation as AnimationType,
  Sprite as SpriteType,
} from '../utils/assetLoader';
import { Sprite } from '../elements/Sprite';
import { animationFrameIndexAtTime } from '../utils/mediaPicker';

interface AnimationProps {
  animation: AnimationType;
  sprites: SpriteType[];
  spriteMap?: Record<string, SpriteType>;
  scale?: number;
  /** Renders each frame into a square canvas of this many CSS pixels. */
  displaySize?: number;
  className?: string;
  autoPlay?: boolean;
  /** Shared elapsed-time clock. When supplied, this preview creates no RAF. */
  clockMs?: number;
}

export function Animation({
  animation,
  sprites,
  spriteMap,
  scale = 1,
  displaySize,
  className = '',
  autoPlay = true,
  clockMs,
}: AnimationProps) {
  const [currentFrameIndex, setCurrentFrameIndex] = useState(0);
  const intervalRef = useRef<number | null>(null);
  const startTimeRef = useRef<number>(0);

  useEffect(() => {
    setCurrentFrameIndex(0);
    startTimeRef.current = performance.now();
  }, [animation.name]);

  // Find sprite by name - use spriteMap if available for O(1) lookup
  const getSpriteByName = (name: string): SpriteType | undefined => {
    if (spriteMap) {
      return spriteMap[name];
    }
    return sprites.find((s) => s.name === name);
  };

  const displayedFrameIndex =
    clockMs === undefined
      ? currentFrameIndex
      : animationFrameIndexAtTime(animation, clockMs);
  const currentFrame = animation.frames[displayedFrameIndex];
  const currentSprite = currentFrame
    ? getSpriteByName(currentFrame.spriteName)
    : undefined;

  useEffect(() => {
    if (clockMs !== undefined || !autoPlay || !currentFrame) {
      return;
    }

    // Clear any existing interval
    if (intervalRef.current !== null) {
      cancelAnimationFrame(intervalRef.current);
    }

    startTimeRef.current = performance.now();

    // Use requestAnimationFrame for smooth animation
    function animate(currentTime: number) {
      if (!currentFrame) return;

      const elapsed = currentTime - startTimeRef.current;
      const frameDuration = currentFrame.frames;

      if (elapsed >= frameDuration) {
        // Move to next frame
        setCurrentFrameIndex((prev) => {
          const next = prev + 1;
          if (next >= animation.frames.length) {
            return animation.loop ? 0 : prev;
          }
          return next;
        });
        startTimeRef.current = currentTime;
      }

      intervalRef.current = requestAnimationFrame(animate);
    }

    intervalRef.current = requestAnimationFrame(animate);

    return () => {
      if (intervalRef.current !== null) {
        cancelAnimationFrame(intervalRef.current);
      }
    };
  }, [currentFrameIndex, currentFrame, animation, autoPlay, clockMs]);

  if (!currentSprite) {
    return (
      <div
        className={className}
        style={{
          display: 'inline-block',
          color: '#858585',
        }}
      >
        Sprite not found: {currentFrame?.spriteName}
      </div>
    );
  }

  return (
    <div className={className} style={{ display: 'inline-block' }}>
      <Sprite sprite={currentSprite} scale={scale} displaySize={displaySize} />
    </div>
  );
}
