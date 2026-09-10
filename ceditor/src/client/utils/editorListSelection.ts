/**
 * Translate an index from a filtered/visible list back to the source list.
 * The editors store source-list indexes so selection remains stable as filters change.
 */
export function sourceIndexFromVisibleIndex<T>(
  source: readonly T[],
  visible: readonly T[],
  visibleIndex: number,
): number {
  const item = visible[visibleIndex];
  return item === undefined ? -1 : source.indexOf(item);
}

/** Return the visible-list index for a source-list selection, or null when filtered out. */
export function visibleIndexFromSourceIndex<T>(
  source: readonly T[],
  visible: readonly T[],
  sourceIndex: number,
): number | null {
  const item = source[sourceIndex];
  if (item === undefined) {
    return null;
  }
  const visibleIndex = visible.indexOf(item);
  return visibleIndex >= 0 ? visibleIndex : null;
}

/** Read the selected source item without translating through the filtered list. */
export function itemAtSourceIndex<T>(
  source: readonly T[],
  sourceIndex: number,
): T | undefined {
  return sourceIndex >= 0 ? source[sourceIndex] : undefined;
}

/** Replace a source-list item while leaving every other item and its index intact. */
export function replaceAtSourceIndex<T>(
  source: readonly T[],
  sourceIndex: number,
  replacement: T,
): T[] {
  if (sourceIndex < 0 || sourceIndex >= source.length) {
    return source.slice();
  }
  const next = source.slice();
  next[sourceIndex] = replacement;
  return next;
}

/** A stable, unique React key for an entity id, including invalid duplicate ids. */
export function recordKeyAtSourceIndex<T>(
  source: readonly T[],
  sourceIndex: number,
  getId: (item: T) => string,
): string {
  const item = source[sourceIndex];
  if (item === undefined) {
    return JSON.stringify(['missing', sourceIndex]);
  }

  const id = getId(item);
  let occurrence = 0;
  for (let index = 0; index < sourceIndex; index += 1) {
    if (getId(source[index]) === id) {
      occurrence += 1;
    }
  }
  return JSON.stringify([id, occurrence]);
}
