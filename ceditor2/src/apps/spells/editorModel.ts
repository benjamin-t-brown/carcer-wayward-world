import type { SpellRecord } from '../../core/domain/spells/index.js';

const DEEP_LINK_KEYS = ['spell', 'selected'] as const;

export function matchesSpellSearch(
  record: SpellRecord,
  searchTerm: string,
): boolean {
  const term = searchTerm.trim().toLocaleLowerCase();
  if (!term) return true;
  return [
    record.name,
    record.label,
    record.description,
    record.abilityName,
  ].some((value) => value.toLocaleLowerCase().includes(term));
}

export function spellNameFromUrl(url: URL): string | null {
  for (const key of DEEP_LINK_KEYS) {
    const value = url.searchParams.get(key)?.trim();
    if (value) return value;
  }

  const hash = url.hash.startsWith('#') ? url.hash.slice(1) : url.hash;
  if (!hash) return null;
  const queryStart = hash.indexOf('?');
  const hashQuery = queryStart >= 0 ? hash.slice(queryStart + 1) : hash;
  const params = new URLSearchParams(hashQuery);
  for (const key of DEEP_LINK_KEYS) {
    const value = params.get(key)?.trim();
    if (value) return value;
  }
  return queryStart < 0 && !hash.includes('=')
    ? decodeURIComponent(hash)
    : null;
}

export function findDeepLinkedSpell(
  records: readonly SpellRecord[],
  url: URL,
): number {
  const name = spellNameFromUrl(url);
  return name ? records.findIndex((record) => record.name === name) : -1;
}

export function withSpellSelection(url: URL, name?: string): URL {
  const next = new URL(url);
  for (const key of DEEP_LINK_KEYS) next.searchParams.delete(key);
  if (name?.trim()) next.searchParams.set('spell', name.trim());
  return next;
}
