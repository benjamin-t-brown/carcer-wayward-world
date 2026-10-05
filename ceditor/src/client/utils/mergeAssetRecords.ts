/**
 * Apply this tab's edits on top of a newer copy of an asset file.
 * Added or changed records replace the disk version. Removed records are
 * dropped. Every other record stays as it is on disk, in disk order.
 */
export function mergeAssetRecords<T extends object>(
  baseline: readonly T[],
  local: readonly T[],
  disk: readonly T[],
  key: string,
): T[] {
  const keyOf = (record: T): string => {
    const value = (record as Record<string, unknown>)[key];
    return typeof value === 'string' ? value : '';
  };
  const baselineJson = new Map(
    baseline.map((record) => [keyOf(record), canonicalJson(record)]),
  );
  const localByKey = new Map(local.map((record) => [keyOf(record), record]));
  const diskKeys = new Set(disk.map((record) => keyOf(record)));

  const merged: T[] = [];
  for (const record of disk) {
    const id = keyOf(record);
    if (baselineJson.has(id) && !localByKey.has(id)) {
      continue;
    }
    const localRecord = localByKey.get(id);
    const previous = baselineJson.get(id);
    if (
      localRecord &&
      (previous === undefined || previous !== canonicalJson(localRecord))
    ) {
      merged.push(localRecord);
    } else {
      merged.push(record);
    }
  }

  for (const record of local) {
    const id = keyOf(record);
    if (diskKeys.has(id)) {
      continue;
    }
    const previous = baselineJson.get(id);
    if (previous === undefined || previous !== canonicalJson(record)) {
      merged.push(record);
    }
  }

  return merged;
}

/** Key order from spreads and trims is not an edit. */
function canonicalJson(value: unknown): string {
  return JSON.stringify(sortKeys(value));
}

function sortKeys(value: unknown): unknown {
  if (Array.isArray(value)) {
    return value.map(sortKeys);
  }
  if (value !== null && typeof value === 'object') {
    const sorted: Record<string, unknown> = {};
    for (const key of Object.keys(value as Record<string, unknown>).sort()) {
      const child = (value as Record<string, unknown>)[key];
      if (child !== undefined) {
        sorted[key] = sortKeys(child);
      }
    }
    return sorted;
  }
  return value;
}
