import { useState } from 'react';
import {
  MEDIA_RENDER_BATCH_SIZE,
  nextMediaRenderLimit,
} from '../utils/mediaPicker';

export function useIncrementalMedia<T>(
  choices: T[],
  resetKey: string,
  batchSize = MEDIA_RENDER_BATCH_SIZE,
) {
  const [state, setState] = useState({ key: resetKey, limit: batchSize });
  const limit = state.key === resetKey ? state.limit : batchSize;
  const visibleChoices = choices.slice(0, limit);
  const hasMore = visibleChoices.length < choices.length;

  const showMore = () => {
    setState({
      key: resetKey,
      limit: nextMediaRenderLimit(limit, choices.length, batchSize),
    });
  };

  return { visibleChoices, hasMore, showMore };
}
