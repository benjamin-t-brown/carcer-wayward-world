export interface ImageResource {
  source: CanvasImageSource;
  width: number;
  height: number;
}

export type ImageLoader = (path: string) => Promise<ImageResource>;

interface ImageCacheEntry {
  resource?: ImageResource;
  error?: Error;
  promise?: Promise<ImageResource>;
}

/** Loads every image path at most once and exposes synchronous frame reads. */
export class ImageCache {
  private readonly entries = new Map<string, ImageCacheEntry>();

  constructor(private readonly loader: ImageLoader = browserImageLoader()) {}

  get(path: string): ImageResource | undefined {
    return this.entries.get(path)?.resource;
  }

  error(path: string): Error | undefined {
    return this.entries.get(path)?.error;
  }

  /** Starts a load when needed; intended for non-awaiting render loops. */
  getOrLoad(path: string): ImageResource | undefined {
    let entry = this.entries.get(path);
    if (!entry) {
      entry = this.createEntry(path);
      this.entries.set(path, entry);
    }
    return entry.resource;
  }

  load(path: string): Promise<ImageResource> {
    let entry = this.entries.get(path);
    if (!entry) {
      entry = this.createEntry(path);
      this.entries.set(path, entry);
    }
    // createEntry always installs the promise before returning.
    return entry.promise as Promise<ImageResource>;
  }

  clear(): void {
    this.entries.clear();
  }

  private createEntry(path: string): ImageCacheEntry {
    const entry: ImageCacheEntry = {};
    entry.promise = Promise.resolve()
      .then(() => this.loader(path))
      .then((resource) => {
        if (resource.width <= 0 || resource.height <= 0) {
          throw new Error(`Image has invalid dimensions: ${path}`);
        }
        entry.resource = resource;
        return resource;
      })
      .catch((cause: unknown) => {
        const error =
          cause instanceof Error
            ? cause
            : new Error(`Failed to load image: ${path}`);
        entry.error = error;
        throw error;
      });
    // A continuous renderer may intentionally start the load without awaiting it.
    void entry.promise.catch(() => undefined);
    return entry;
  }
}

export function gameAssetUrl(path: string): string {
  const relativePath = path
    .replace(/^(?:\.\/|\/)+/, '')
    .replace(/^assets\//, '');
  return `/game-assets/${relativePath}`;
}

export function browserImageLoader(
  resolveUrl: (path: string) => string = gameAssetUrl,
): ImageLoader {
  return (path) =>
    new Promise<ImageResource>((resolve, reject) => {
      const image = new Image();
      image.onload = () => {
        resolve({
          source: image,
          width: image.naturalWidth || image.width,
          height: image.naturalHeight || image.height,
        });
      };
      image.onerror = () => {
        reject(new Error(`Failed to load image: ${path}`));
      };
      image.src = resolveUrl(path);
    });
}
