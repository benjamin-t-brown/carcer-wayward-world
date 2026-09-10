import type { CharacterRecord } from '../../core/domain/characters/index.js';

const DEEP_LINK_KEYS = ['character', 'selected'] as const;

export function matchesCharacterSearch(
  record: CharacterRecord,
  searchTerm: string,
): boolean {
  const term = searchTerm.trim().toLocaleLowerCase();
  if (!term) return true;
  return [record.name, record.label, record.type].some((value) =>
    value.toLocaleLowerCase().includes(term),
  );
}

export function characterNameFromUrl(url: URL): string | null {
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

export function findDeepLinkedCharacter(
  records: readonly CharacterRecord[],
  url: URL,
): number {
  const name = characterNameFromUrl(url);
  return name ? records.findIndex((record) => record.name === name) : -1;
}

export function withCharacterSelection(url: URL, name?: string): URL {
  const next = new URL(url);
  DEEP_LINK_KEYS.forEach((key) => next.searchParams.delete(key));
  if (name?.trim()) next.searchParams.set('character', name.trim());
  return next;
}
