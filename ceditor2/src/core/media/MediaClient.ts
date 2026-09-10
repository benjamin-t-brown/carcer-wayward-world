import { createMediaCatalog } from './assetFileParser.js';
import type { MediaCatalog, MediaSourceFiles, SoundFileMode } from './types.js';

export async function loadMediaCatalog(
  options: {
    baseUrl?: string;
    fetch?: typeof globalThis.fetch;
    soundMode?: SoundFileMode;
  } = {},
): Promise<MediaCatalog> {
  const fetchImplementation = options.fetch ?? globalThis.fetch;
  if (!fetchImplementation) {
    throw new Error('The Fetch API is not available');
  }

  const baseUrl = options.baseUrl?.replace(/\/$/, '') ?? '';
  const response = await fetchImplementation(`${baseUrl}/api/media-sources`);
  if (!response.ok) {
    throw new Error(
      response.statusText ||
        `Failed to load media sources (${response.status})`,
    );
  }

  const sources = (await response.json()) as unknown;
  if (!isMediaSourceFiles(sources)) {
    throw new Error('Media source response must be a filename-to-text object');
  }

  return createMediaCatalog(sources, options.soundMode);
}

function isMediaSourceFiles(value: unknown): value is MediaSourceFiles {
  return (
    typeof value === 'object' &&
    value !== null &&
    !Array.isArray(value) &&
    Object.values(value).every((content) => typeof content === 'string')
  );
}
