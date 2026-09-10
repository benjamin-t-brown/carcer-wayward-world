import { useEffect, useState } from 'react';

/** One shared animation clock for every preview within an open picker. */
export function useAnimationClock(active: boolean): number {
  const [elapsedMs, setElapsedMs] = useState(0);

  useEffect(() => {
    if (!active) {
      setElapsedMs(0);
      return;
    }

    const startedAt = performance.now();
    let frameId = 0;
    const tick = (now: number) => {
      setElapsedMs(now - startedAt);
      frameId = requestAnimationFrame(tick);
    };
    frameId = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(frameId);
  }, [active]);

  return elapsedMs;
}
