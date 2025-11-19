import React, { useEffect } from 'react';

const useResize = (cb: (width: number, height: number) => void) => {
  return useEffect(() => {
    let timeoutId: number | undefined;

    const handleResize = () => {
      if (timeoutId !== undefined) {
        clearTimeout(timeoutId);
      }
      timeoutId = window.setTimeout(() => {
        cb(window.innerWidth, window.innerHeight);
      }, 300);
    };

    window.addEventListener('resize', handleResize);
    return () => {
      window.removeEventListener('resize', handleResize);
      if (timeoutId !== undefined) {
        clearTimeout(timeoutId);
      }
    };
  }, [cb]);
};

export const MapCanvas = (props: {
  width: number;
  height: number;
  canvasRef: React.RefObject<HTMLCanvasElement>;
}) => {
  useResize((width, height) => {
    const canvas = props.canvasRef.current;
    if (canvas) {
      canvas.width = width;
      canvas.height = height;
    }
  });
  return (
    <div
      id="map-canvas"
      style={{
        width: '100%',
        height: '100%',
        overflow: 'hidden',
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
        imageRendering: 'pixelated',
      }}
    >
      <canvas
        id="map-canvas-canvas"
        ref={props.canvasRef}
        width={props.width}
        height={props.height}
      />
    </div>
  );
};
