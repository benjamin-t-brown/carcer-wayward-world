import assert from 'node:assert/strict';
import test from 'node:test';

import type { ItemTemplate } from '../client/types/assets';
import { validateItemTemplatesBeforeSave } from '../client/pages/itemTemplatesModel';

function item(overrides: Partial<ItemTemplate> = {}): ItemTemplate {
  return {
    itemType: 'UTILITY',
    name: 'item',
    label: 'Item',
    icon: 'items_0',
    description: 'Description',
    weight: 1,
    value: 1,
    ...overrides,
  };
}

test('item validation accepts complete records without changing them', () => {
  const items = [
    item({
      futureItemRule: { note: 'preserved' },
    } as Partial<ItemTemplate>),
  ];
  const before = structuredClone(items);

  assert.equal(validateItemTemplatesBeforeSave(items), null);
  assert.deepEqual(items, before);
});

test('item validation preserves existing duplicate and required-field errors', () => {
  const error = validateItemTemplatesBeforeSave([
    item({ name: 'duplicate' }),
    item({
      name: 'duplicate',
      label: '',
      icon: '',
      description: '',
      weight: Number.NaN,
      value: Number.NaN,
    }),
  ]);

  assert.equal(
    error,
    [
      'Duplicate item names found: duplicate',
      'Items with missing required fields:\nduplicate: missing label, icon, description, weight, value',
    ].join('\n\n'),
  );
});
