import React, { useEffect } from 'react';

export const MapCanvas = (props: {
  width: number;
  height: number;
  canvasRef: React.RefObject<HTMLCanvasElement>;
}) => {
  useEffect(() => {
    const canvas = props.canvasRef.current;
    const container = canvas?.parentElement;
    if (!canvas || !container) return;

    const resize = () => {
      const width = container.clientWidth || props.width;
      const height = container.clientHeight || props.height;
      if (canvas.width !== width) canvas.width = width;
      if (canvas.height !== height) canvas.height = height;
      const ctx = canvas.getContext('2d');
      if (ctx) ctx.imageSmoothingEnabled = false;
      canvas.style.imageRendering = 'pixelated';
    };

    resize();
    const observer = new ResizeObserver(resize);
    observer.observe(container);
    return () => observer.disconnect();
  }, [props.canvasRef, props.height, props.width]);
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
