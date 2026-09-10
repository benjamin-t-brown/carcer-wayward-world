import assert from 'node:assert/strict';
import test from 'node:test';

import type { ItemRecord } from '../../src/core/domain/items/index.js';
import {
  findDeepLinkedItem,
  itemNameFromUrl,
  matchesItemSearch,
  withItemSelection,
} from '../../src/apps/items/editorModel.js';

const records: ItemRecord[] = [
  {
    itemType: 'POTION',
    name: 'PotionHealing',
    label: 'Healing Potion',
    icon: 'potion',
    description: 'Restores health.',
    weight: 1,
    value: 10,
  },
  {
    itemType: 'WEAPON_MELEE',
    name: 'DaggerBronze',
    label: 'Bronze Dagger',
    icon: 'dagger',
    description: 'An edged weapon.',
    weight: 2,
    value: 5,
  },
];

test('item search covers identity, prose, and item type', () => {
  assert.equal(matchesItemSearch(records[0]!, 'healing'), true);
  assert.equal(matchesItemSearch(records[0]!, 'RESTORES HEALTH'), true);
  assert.equal(matchesItemSearch(records[1]!, 'weapon_melee'), true);
  assert.equal(matchesItemSearch(records[0]!, 'dagger'), false);
  assert.equal(matchesItemSearch(records[0]!, '  '), true);
});

test('reads canonical and legacy item deep links', () => {
  assert.equal(
    itemNameFromUrl(
      new URL('http://localhost/pages/items/?item=PotionHealing'),
    ),
    'PotionHealing',
  );
  assert.equal(
    itemNameFromUrl(
      new URL('http://localhost/#/editor/itemTemplates?item=DaggerBronze'),
    ),
    'DaggerBronze',
  );
  assert.equal(
    itemNameFromUrl(new URL('http://localhost/pages/items/#PotionHealing')),
    'PotionHealing',
  );
});

test('selects and writes a stable item deep link', () => {
  assert.equal(
    findDeepLinkedItem(records, new URL('http://localhost/?item=DaggerBronze')),
    1,
  );
  assert.equal(
    findDeepLinkedItem(records, new URL('http://localhost/?item=MISSING')),
    -1,
  );
  const next = withItemSelection(
    new URL('http://localhost/pages/items/?debug=1&selected=OLD#section'),
    'Potion Healing',
  );
  assert.equal(next.searchParams.get('debug'), '1');
  assert.equal(next.searchParams.get('selected'), null);
  assert.equal(next.searchParams.get('item'), 'Potion Healing');
  assert.equal(next.hash, '#section');
});
