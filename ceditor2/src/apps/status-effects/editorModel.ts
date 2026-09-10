import type { StatusEffectRecord } from '../../core/domain/statusEffects/index.js';

const DEEP_LINK_KEYS = ['statusEffect', 'effect', 'selected'] as const;

export function matchesStatusEffectSearch(
  record: StatusEffectRecord,
  searchTerm: string,
): boolean {
  const term = searchTerm.trim().toLocaleLowerCase();
  if (!term) {
    return true;
  }

  return [record.name, record.description].some((value) =>
    value.toLocaleLowerCase().includes(term),
  );
}

export function statusEffectNameFromUrl(url: URL): string | null {
  for (const key of DEEP_LINK_KEYS) {
    const value = url.searchParams.get(key)?.trim();
    if (value) {
      return value;
    }
  }

  const hash = url.hash.startsWith('#') ? url.hash.slice(1) : url.hash;
  if (!hash) {
    return null;
  }

  const queryStart = hash.indexOf('?');
  const hashQuery = queryStart >= 0 ? hash.slice(queryStart + 1) : hash;
  const hashParams = new URLSearchParams(hashQuery);
  for (const key of DEEP_LINK_KEYS) {
    const value = hashParams.get(key)?.trim();
    if (value) {
      return value;
    }
  }

  return queryStart < 0 && !hash.includes('=')
    ? decodeURIComponent(hash)
    : null;
}

export function findDeepLinkedStatusEffect(
  records: readonly StatusEffectRecord[],
  url: URL,
): number {
  const selectedName = statusEffectNameFromUrl(url);
  return selectedName
    ? records.findIndex((record) => record.name === selectedName)
    : -1;
}

export function withStatusEffectSelection(url: URL, name?: string): URL {
  const next = new URL(url);
  for (const key of DEEP_LINK_KEYS) {
    next.searchParams.delete(key);
  }

  if (name?.trim()) {
    next.searchParams.set('statusEffect', name.trim());
  }
  return next;
}
