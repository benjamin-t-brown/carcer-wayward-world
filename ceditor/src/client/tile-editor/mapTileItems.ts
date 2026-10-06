import { CarcerMapTemplate, MapItemPlacement, MapTileItemEntry } from '../types/assets';

export function clampMapTileItemQuantity(quantity: number): number {
  return Math.max(1, Math.floor(quantity));
}

function isDropTableEntry(entry: MapTileItemEntry): boolean {
  return Boolean(entry.dropTable?.trim());
}

/** Coerces legacy string / missing-quantity entries when loading maps. */
export function coerceMapTileItemEntry(raw: unknown): MapTileItemEntry {
  if (typeof raw === 'string') {
    return { name: raw, quantity: 1 };
  }
  if (raw && typeof raw === 'object') {
    const obj = raw as Record<string, unknown>;
    const dropTable =
      typeof obj.dropTable === 'string' ? obj.dropTable.trim() : '';
    if (dropTable) {
      return { dropTable };
    }
    if (typeof obj.name === 'string' && obj.name) {
      return {
        name: obj.name,
        quantity: clampMapTileItemQuantity(
          typeof obj.quantity === 'number' ? obj.quantity : 1,
        ),
      };
    }
  }
  throw new Error('Invalid map tile item entry');
}

function normalizeMapItemPlacement(item: MapItemPlacement): MapItemPlacement {
  const dropTable = item.dropTable?.trim();
  if (dropTable) {
    return { l: item.l, i: item.i, dropTable };
  }
  const name = item.name?.trim();
  if (!name) {
    return item;
  }
  return {
    l: item.l,
    i: item.i,
    name,
    quantity: clampMapTileItemQuantity(item.quantity ?? 1),
  };
}

export function normalizeMapItemsOnLoad(
  maps: CarcerMapTemplate[],
): CarcerMapTemplate[] {
  for (const map of maps) {
    map.items = (map.items ?? []).map((item) => normalizeMapItemPlacement(item));
  }
  return maps;
}

export function hasMapTileItem(
  items: MapTileItemEntry[],
  itemName: string,
): boolean {
  return items.some(
    (entry) => !isDropTableEntry(entry) && entry.name === itemName,
  );
}

export function addMapTileItemEntry(
  items: MapTileItemEntry[],
  itemName: string,
  stackable: boolean,
): MapTileItemEntry[] {
  if (hasMapTileItem(items, itemName)) {
    if (stackable) {
      const index = items.findIndex(
        (e) => !isDropTableEntry(e) && e.name === itemName,
      );
      if (index >= 0) {
        const current = items[index]!;
        return setMapTileItemQuantityAtIndex(
          items,
          index,
          (current.quantity ?? 1) + 1,
        );
      }
    }
    return items;
  }
  return [...items, { name: itemName, quantity: 1 }];
}

export function addMapTileDropTableEntry(
  items: MapTileItemEntry[],
  dropTableName: string,
): MapTileItemEntry[] {
  const dropTable = dropTableName.trim();
  if (!dropTable) {
    return items;
  }
  return [...items, { dropTable }];
}

export function setMapTileItemQuantity(
  items: MapTileItemEntry[],
  itemName: string,
  quantity: number,
): MapTileItemEntry[] {
  const index = items.findIndex(
    (entry) => !isDropTableEntry(entry) && entry.name === itemName,
  );
  if (index < 0) {
    return items;
  }
  return setMapTileItemQuantityAtIndex(items, index, quantity);
}

export function setMapTileItemQuantityAtIndex(
  items: MapTileItemEntry[],
  index: number,
  quantity: number,
): MapTileItemEntry[] {
  if (index < 0 || index >= items.length) {
    return items;
  }
  const entry = items[index]!;
  if (isDropTableEntry(entry)) {
    return items;
  }
  const q = clampMapTileItemQuantity(quantity);
  return items.map((e, i) =>
    i !== index ? e : { name: entry.name, quantity: q },
  );
}

export function removeMapTileItem(
  items: MapTileItemEntry[],
  itemName: string,
): MapTileItemEntry[] {
  return items.filter(
    (entry) => isDropTableEntry(entry) || entry.name !== itemName,
  );
}

export function removeMapTileItemAtIndex(
  items: MapTileItemEntry[],
  index: number,
): MapTileItemEntry[] {
  if (index < 0 || index >= items.length) {
    return items;
  }
  return items.filter((_, i) => i !== index);
}

export function moveMapTileItem(
  items: MapTileItemEntry[],
  index: number,
  direction: 'up' | 'down',
): MapTileItemEntry[] {
  const targetIndex = direction === 'up' ? index - 1 : index + 1;
  if (targetIndex < 0 || targetIndex >= items.length) {
    return items;
  }
  const next = [...items];
  const [entry] = next.splice(index, 1);
  next.splice(targetIndex, 0, entry);
  return next;
}
