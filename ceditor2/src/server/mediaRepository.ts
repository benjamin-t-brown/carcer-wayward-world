import { readdir, readFile } from 'node:fs/promises';
import { join } from 'node:path';

import type { MediaSourceFiles } from '../core/media/types.js';

export interface MediaSourceRepositoryContract {
  load(): Promise<MediaSourceFiles>;
}

export class MediaSourceRepository implements MediaSourceRepositoryContract {
  constructor(private readonly assetsPath: string) {}

  async load(): Promise<MediaSourceFiles> {
    const filenames = (await readdir(this.assetsPath))
      .filter(
        (filename) =>
          filename.startsWith('assets.') && filename.endsWith('.txt'),
      )
      .sort();
    const sources = await Promise.all(
      filenames.map(
        async (filename) =>
          [
            filename,
            await readFile(join(this.assetsPath, filename), 'utf8'),
          ] as const,
      ),
    );
    return Object.fromEntries(sources);
  }
}
