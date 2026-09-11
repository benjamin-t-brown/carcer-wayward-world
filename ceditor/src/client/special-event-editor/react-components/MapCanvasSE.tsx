import React, { useEffect } from 'react';

export const CANVAS_CONTAINER_ID = 'special-event-editor-canvas';

export const MapCanvasSE = (props: {
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
      canvas.style.imageRendering = 'pixelated';
      const ctx = canvas.getContext('2d');
      if (ctx) ctx.imageSmoothingEnabled = false;
    };

    resize();
    const observer = new ResizeObserver(resize);
    observer.observe(container);
    return () => observer.disconnect();
  }, [props.canvasRef, props.height, props.width]);
  return (
    <div
      id={CANVAS_CONTAINER_ID}
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
      <canvas id={CANVAS_CONTAINER_ID + '-canvas'} ref={props.canvasRef} />
    </div>
  );
};
