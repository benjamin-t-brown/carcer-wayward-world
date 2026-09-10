import type { ItemTemplate } from '../types/assets';

export function validateItemTemplatesBeforeSave(
  items: ItemTemplate[],
): string | null {
  const nameCounts = new Map<string, number>();
  const itemsWithMissingFields: string[] = [];

  items.forEach((item, index) => {
    const missingFields: string[] = [];

    if (!item.itemType || item.itemType.trim() === '') {
      missingFields.push('itemType');
    }
    if (!item.name || item.name.trim() === '') {
      missingFields.push('name');
    }
    if (!item.label || item.label.trim() === '') {
      missingFields.push('label');
    }
    if (!item.icon || item.icon.trim() === '') {
      missingFields.push('icon');
    }
    if (!item.description || item.description.trim() === '') {
      missingFields.push('description');
    }
    if (
      item.weight === undefined ||
      item.weight === null ||
      isNaN(item.weight)
    ) {
      missingFields.push('weight');
    }
    if (item.value === undefined || item.value === null || isNaN(item.value)) {
      missingFields.push('value');
    }

    if (missingFields.length > 0) {
      const itemIdentifier = item.name || `Item at index ${index}`;
      itemsWithMissingFields.push(
        `${itemIdentifier}: missing ${missingFields.join(', ')}`,
      );
    }

    if (item.name && item.name.trim()) {
      const count = nameCounts.get(item.name) || 0;
      nameCounts.set(item.name, count + 1);
    }
  });

  const errors: string[] = [];
  const duplicateNames: string[] = [];
  nameCounts.forEach((count, name) => {
    if (count > 1) {
      duplicateNames.push(name);
    }
  });

  if (duplicateNames.length > 0) {
    errors.push(`Duplicate item names found: ${duplicateNames.join(', ')}`);
  }
  if (itemsWithMissingFields.length > 0) {
    errors.push(
      `Items with missing required fields:\n${itemsWithMissingFields.join('\n')}`,
    );
  }

  return errors.length > 0 ? errors.join('\n\n') : null;
}
