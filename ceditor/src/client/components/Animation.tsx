import { useState, useEffect, useRef } from 'react';
import { Animation as AnimationType, Sprite as SpriteType } from '../types/assetsSDL2w';
import { Sprite } from '../elements/Sprite';

interface AnimationProps {
  animation: AnimationType;
  sprites: SpriteType[];
  spriteMap?: Record<string, SpriteType>;
  scale?: number;
  className?: string;
  autoPlay?: boolean;
}

export function Animation({
  animation,
  sprites,
  spriteMap,
  scale = 1,
  className = '',
  autoPlay = true,
}: AnimationProps) {
  const [currentFrameIndex, setCurrentFrameIndex] = useState(0);
  const intervalRef = useRef<number | null>(null);
  const startTimeRef = useRef<number>(0);

  // Find sprite by name - use spriteMap if available for O(1) lookup
  const getSpriteByName = (name: string): SpriteType | undefined => {
    if (spriteMap) {
      return spriteMap[name];
    }
    return sprites.find((s) => s.name === name);
  };

  const currentFrame = animation.frames[currentFrameIndex];
  const currentSprite = currentFrame
    ? getSpriteByName(currentFrame.spriteName)
    : undefined;

  useEffect(() => {
    if (!autoPlay || !currentFrame) {
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
      const frameDuration = currentFrame.frames * 16; // Convert game frames to ms (assuming 60fps)

      if (elapsed >= frameDuration) {
        // Move to next frame
        setCurrentFrameIndex((prev) => {
          const next = prev + 1;
          if (next >= animation.frames.length) {
            // Loop or stop
            if (animation.loop) {
              return 0;
            }
            return prev;
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
  }, [currentFrameIndex, currentFrame, animation, autoPlay]);

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
      <Sprite sprite={currentSprite} scale={scale} />
    </div>
  );
}

