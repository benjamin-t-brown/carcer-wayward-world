import { CarcerMapTemplate, MapTileItemEntry } from '../types/assets';

export function clampMapTileItemQuantity(quantity: number): number {
  return Math.max(1, Math.floor(quantity));
}

/** Coerces legacy string / missing-quantity entries when loading maps. */
export function coerceMapTileItemEntry(raw: unknown): MapTileItemEntry {
  if (typeof raw === 'string') {
    return { name: raw, quantity: 1 };
  }
  if (raw && typeof raw === 'object' && 'name' in raw) {
    const entry = raw as { name: string; quantity?: number };
    return {
      name: entry.name,
      quantity: clampMapTileItemQuantity(
        typeof entry.quantity === 'number' ? entry.quantity : 1
      ),
    };
  }
  throw new Error('Invalid map tile item entry');
}

export function normalizeMapItemsOnLoad(
  maps: CarcerMapTemplate[]
): CarcerMapTemplate[] {
  for (const map of maps) {
    map.items = (map.items ?? []).map((item) => ({
      ...item,
      quantity: clampMapTileItemQuantity(item.quantity ?? 1),
    }));
  }
  return maps;
}

export function hasMapTileItem(
  items: MapTileItemEntry[],
  itemName: string
): boolean {
  return items.some((entry) => entry.name === itemName);
}

export function addMapTileItemEntry(
  items: MapTileItemEntry[],
  itemName: string,
  stackable: boolean
): MapTileItemEntry[] {
  if (hasMapTileItem(items, itemName)) {
    if (stackable) {
      const current = items.find((e) => e.name === itemName)!;
      return setMapTileItemQuantity(items, itemName, current.quantity + 1);
    }
    return items;
  }
  return [...items, { name: itemName, quantity: 1 }];
}

export function setMapTileItemQuantity(
  items: MapTileItemEntry[],
  itemName: string,
  quantity: number
): MapTileItemEntry[] {
  const q = clampMapTileItemQuantity(quantity);
  return items.map((entry) =>
    entry.name !== itemName ? entry : { name: itemName, quantity: q }
  );
}

export function removeMapTileItem(
  items: MapTileItemEntry[],
  itemName: string
): MapTileItemEntry[] {
  return items.filter((entry) => entry.name !== itemName);
}

export function moveMapTileItem(
  items: MapTileItemEntry[],
  index: number,
  direction: 'up' | 'down'
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
