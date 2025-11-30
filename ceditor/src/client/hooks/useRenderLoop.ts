import { useEffect, useRef } from 'react';

export function useRenderLoop(callback: (ts: number) => void) {
  const callbackRef = useRef(callback);
  const frameRef = useRef<number | null>(null);

  useEffect(() => {
    callbackRef.current = callback;
  }, [callback]);

  // useEffect(() => {
  //   const loop = (ts: number) => {
  //     callbackRef.current(ts);
  //     frameRef.current = requestAnimationFrame(loop);
  //   };

  //   frameRef.current = requestAnimationFrame(loop);

  //   return () => {
  //     if (frameRef.current !== null) {
  //       cancelAnimationFrame(frameRef.current);
  //     }
  //   };
  // }, []);

  // for debug
  useEffect(() => {
    const loop = (ts: number) => {
      callbackRef.current(ts);
      // frameRef.current = requestAnimationFrame(loop);
    };

    frameRef.current = setInterval(() => {
      loop(performance.now());
    }, 100) as unknown as number;

    return () => {
      if (frameRef.current !== null) {
        clearInterval(frameRef.current);
      }
    };
  }, []);
}
