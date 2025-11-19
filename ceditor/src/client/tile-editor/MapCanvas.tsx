import React, { useEffect } from 'react';

const useResize = (cb: (width: number, height: number) => void) => {
  return useEffect(() => {
    const handleResize = () => {
      cb(window.innerWidth, window.innerHeight);
    };
    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, [cb]);
};

export const MapCanvas = (props: {
  width: number;
  height: number;
  canvasRef: React.RefObject<HTMLCanvasElement>;
}) => {
  // const canvasRef = React.useRef<HTMLCanvasElement>(null);
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
        // transformOrigin: '50% 50%',
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
