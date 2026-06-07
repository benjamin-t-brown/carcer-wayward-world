import { useEffect, useRef } from 'react';
import { CarcerMapTemplate } from '../types/assets';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import {
  MAP_PREVIEW_DISPLAY_SIZE,
  drawMapPreviewToCanvas,
} from '../utils/mapPreview';

interface MapPreviewProps {
  map: CarcerMapTemplate;
  displaySize?: number;
  className?: string;
}

export function MapPreview({
  map,
  displaySize = MAP_PREVIEW_DISPLAY_SIZE,
  className = '',
}: MapPreviewProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const { spriteMap } = useSDL2WAssets();

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) {
      return;
    }
    drawMapPreviewToCanvas(canvas, map, spriteMap, displaySize);
  }, [map, spriteMap, displaySize]);

  return (
    <canvas
      ref={canvasRef}
      className={className}
      style={{
        width: displaySize,
        height: displaySize,
        imageRendering: 'pixelated',
        display: 'block',
        backgroundColor: '#421',
        borderRadius: '4px',
      }}
    />
  );
}
