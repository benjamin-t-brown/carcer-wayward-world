import assert from 'node:assert/strict';
import test from 'node:test';

import type { AbilityTemplate } from '../client/types/ability';
import type {
  CharacterTemplate,
  ItemTemplate,
  MapGridTemplate,
  TilesetTemplate,
} from '../client/types/assets';
import {
  prepareAbilitiesForSave,
  prepareCharactersForSave,
  prepareItemsForSave,
  prepareMapGridsForSave,
  prepareTemplateRecordsForSave,
  prepareTilesetsForSave,
} from '../client/utils/formSavePreparation';

test('ability preparation trims a deep copy and sorts by name', () => {
  const abilities = [
    {
      name: ' beta ',
      label: ' Beta ',
      description: ' second ',
    },
    {
      name: ' alpha ',
      label: ' Alpha ',
      description: ' first ',
    },
  ] as AbilityTemplate[];
  const before = structuredClone(abilities);

  const prepared = prepareAbilitiesForSave(abilities);

  assert.deepEqual(
    prepared.map(({ name, label, description }) => ({
      name,
      label,
      description,
    })),
    [
      { name: 'alpha', label: 'Alpha', description: 'first' },
      { name: 'beta', label: 'Beta', description: 'second' },
    ],
  );
  assert.deepEqual(abilities, before);
  assert.notEqual(prepared, abilities);
  assert.notEqual(prepared[0], abilities[1]);
});

test('item and character preparation use label as the equal-name tie breaker', () => {
  const items = [
    { name: ' same ', label: ' Zebra ' },
    {
      name: ' same ',
      label: ' Alpha ',
      futureItemRule: { note: ' retained ' },
    },
    { name: ' before ', label: ' Before ' },
  ] as ItemTemplate[];
  const characters = [
    { name: ' same ', label: ' Zebra ' },
    { name: ' same ', label: ' Alpha ' },
  ] as CharacterTemplate[];

  const preparedItems = prepareItemsForSave(items);
  const preparedCharacters = prepareCharactersForSave(characters);

  assert.deepEqual(
    preparedItems.map(({ name, label }) => [name, label]),
    [
      ['before', 'Before'],
      ['same', 'Alpha'],
      ['same', 'Zebra'],
    ],
  );
  assert.deepEqual(
    preparedCharacters.map(({ name, label }) => [name, label]),
    [
      ['same', 'Alpha'],
      ['same', 'Zebra'],
    ],
  );
  assert.equal(items[0]?.name, ' same ');
  assert.deepEqual(
    (
      preparedItems[1] as ItemTemplate & {
        futureItemRule: { note: string };
      }
    ).futureItemRule,
    { note: 'retained' },
  );
  assert.deepEqual(
    (
      items[1] as ItemTemplate & {
        futureItemRule: { note: string };
      }
    ).futureItemRule,
    { note: ' retained ' },
  );
  assert.equal(characters[0]?.label, ' Zebra ');
});

test('tileset preparation retains input order when trimmed names tie', () => {
  const tilesets = [
    { name: ' same ', spriteBase: ' first ' },
    { name: 'same', spriteBase: ' second ' },
  ] as TilesetTemplate[];

  const prepared = prepareTilesetsForSave(tilesets);

  assert.deepEqual(
    prepared.map(({ name, spriteBase }) => [name, spriteBase]),
    [
      ['same', 'first'],
      ['same', 'second'],
    ],
  );
  assert.equal(tilesets[0]?.spriteBase, ' first ');
});

test('map-grid preparation trims, sanitizes, sorts, and preserves unknown fields', () => {
  const grids = [
    {
      name: ' z-grid ',
      label: ' Z ',
      gridWidth: 1.9,
      gridHeight: 1,
      mapWidth: 25.8,
      mapHeight: 20,
      cells: [[' partition-a ', 'discarded']],
      futureGridRule: { note: ' keep me ' },
    },
    {
      name: ' a-grid ',
      label: ' A ',
      gridWidth: 1,
      gridHeight: 1,
      mapWidth: 25,
      mapHeight: 20,
      cells: [[' partition-b ']],
    },
  ] as unknown as MapGridTemplate[];
  const before = structuredClone(grids);

  const prepared = prepareMapGridsForSave(grids);

  assert.deepEqual(
    prepared.map(({ name }) => name),
    ['a-grid', 'z-grid'],
  );
  const zGrid = prepared[1] as MapGridTemplate & {
    futureGridRule: { note: string };
  };
  assert.equal(zGrid.gridWidth, 1);
  assert.equal(zGrid.mapWidth, 25);
  assert.deepEqual(zGrid.cells, [['partition-a']]);
  assert.deepEqual(zGrid.futureGridRule, { note: 'keep me' });
  assert.deepEqual(grids, before);
});

test('generic preparation uses the supplied comparator after trimming records', () => {
  const records = [
    { id: ' beta ', order: 1, nested: { note: ' second ' } },
    { id: ' alpha ', order: 2, nested: { note: ' first ' } },
  ];
  const before = structuredClone(records);

  const prepared = prepareTemplateRecordsForSave(
    records,
    (record) => record.id,
    (a, b) => b.order - a.order,
  );

  assert.deepEqual(
    prepared.map(({ id }) => id),
    ['alpha', 'beta'],
  );
  assert.equal(prepared[0]?.nested.note, 'first');
  assert.deepEqual(records, before);
});

test('generic preparation defaults to trimmed id order', () => {
  const prepared = prepareTemplateRecordsForSave(
    [{ id: ' z ' }, { id: ' a ' }],
    (record) => record.id,
  );

  assert.deepEqual(
    prepared.map(({ id }) => id),
    ['a', 'z'],
  );
});
