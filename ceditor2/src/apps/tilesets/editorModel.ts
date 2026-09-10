import type { TilesetRecord } from '../../core/domain/tilesets/index.js';

const DEEP_LINK_KEYS = ['tileset', 'selected'] as const;

export function matchesTilesetSearch(
  record: TilesetRecord,
  searchTerm: string,
): boolean {
  const term = searchTerm.trim().toLocaleLowerCase();
  if (!term) return true;
  return [record.name, record.spriteBase ?? ''].some((value) =>
    value.toLocaleLowerCase().includes(term),
  );
}

export function tilesetNameFromUrl(url: URL): string | null {
  for (const key of DEEP_LINK_KEYS) {
    const value = url.searchParams.get(key)?.trim();
    if (value) return value;
  }
  const hash = url.hash.startsWith('#') ? url.hash.slice(1) : url.hash;
  if (!hash) return null;
  const queryStart = hash.indexOf('?');
  const params = new URLSearchParams(
    queryStart >= 0 ? hash.slice(queryStart + 1) : hash,
  );
  for (const key of DEEP_LINK_KEYS) {
    const value = params.get(key)?.trim();
    if (value) return value;
  }
  return queryStart < 0 && !hash.includes('=')
    ? decodeURIComponent(hash)
    : null;
}

export function findDeepLinkedTileset(
  records: readonly TilesetRecord[],
  url: URL,
): number {
  const name = tilesetNameFromUrl(url);
  return name ? records.findIndex((record) => record.name === name) : -1;
}

export function withTilesetSelection(url: URL, name?: string): URL {
  const next = new URL(url);
  for (const key of DEEP_LINK_KEYS) next.searchParams.delete(key);
  if (name?.trim()) next.searchParams.set('tileset', name.trim());
  return next;
}
