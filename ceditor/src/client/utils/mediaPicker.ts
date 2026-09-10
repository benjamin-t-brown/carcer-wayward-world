import type { Animation } from './assetLoader';

export const MEDIA_RENDER_BATCH_SIZE = 72;

export function filterMediaChoices<T>(
  choices: T[],
  search: string,
  searchValues: (choice: T) => Array<string | undefined>,
): T[] {
  const term = search.trim().toLocaleLowerCase();
  if (!term) {
    return choices;
  }
  return choices.filter((choice) =>
    searchValues(choice).some((value) =>
      value?.toLocaleLowerCase().includes(term),
    ),
  );
}

export function nextMediaRenderLimit(
  currentLimit: number,
  total: number,
  batchSize = MEDIA_RENDER_BATCH_SIZE,
): number {
  return Math.min(total, Math.max(0, currentLimit) + Math.max(1, batchSize));
}

/** Resolve one animation frame from a shared elapsed-time clock. */
export function animationFrameIndexAtTime(
  animation: Animation,
  elapsedMs: number,
): number {
  if (animation.frames.length === 0) {
    return 0;
  }
  const totalDuration = animation.frames.reduce(
    (total, frame) => total + Math.max(1, frame.frames),
    0,
  );
  let remaining = animation.loop
    ? Math.max(0, elapsedMs) % totalDuration
    : Math.min(Math.max(0, elapsedMs), totalDuration - 1);

  for (let index = 0; index < animation.frames.length; index += 1) {
    const duration = Math.max(1, animation.frames[index]?.frames ?? 1);
    if (remaining < duration) {
      return index;
    }
    remaining -= duration;
  }
  return animation.frames.length - 1;
}
