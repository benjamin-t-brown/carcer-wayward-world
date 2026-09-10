import assert from 'node:assert/strict';
import test from 'node:test';
import type {
  DatabaseSnapshot,
  JsonObject,
} from '../../src/core/database/index.js';
import { buildReferenceIndex } from '../../src/core/references/index.js';
import { validateDatabase } from '../../src/core/validation/index.js';

function snapshot(overrides: Partial<DatabaseSnapshot> = {}): DatabaseSnapshot {
  return {
    statusEffects: [],
    abilities: [],
    items: [],
    spells: [],
    characters: [],
    maps: [],
    mapGrids: [],
    tilesets: [],
    specialEvents: [],
    ...overrides,
  };
}

function map(overrides: JsonObject = {}): JsonObject {
  return {
    name: 'map-one',
    width: 1,
    height: 1,
    layers: [0],
    tilesets: [''],
    tiles: { 0: [0, 0] },
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
    ...overrides,
  };
}

test('an empty snapshot has no structural or reference issues', () => {
  const result = validateDatabase(snapshot());

  assert.equal(result.valid, true);
  assert.deepEqual(result.issues, []);
});

test('requires every collection to be an array and every record to be an object', () => {
  const malformed = {
    ...snapshot(),
    items: null,
    abilities: ['not-an-object'],
  };
  const result = validateDatabase(malformed);

  assert.equal(result.valid, false);
  assert.ok(
    result.errors.some((issue) => issue.code === 'collection.root.array'),
  );
  assert.ok(result.errors.some((issue) => issue.code === 'record.object'));
});

test('requires stable identities and rejects duplicates in all collections', () => {
  const result = validateDatabase(
    snapshot({
      items: [{ name: 'same' }, { name: 'same' }, { label: 'missing name' }],
      specialEvents: [
        { id: 'event' },
        { id: 'event' },
        { title: 'missing id' },
      ],
    }),
  );

  assert.equal(
    result.errors.filter((issue) => issue.code === 'record.identity.duplicate')
      .length,
    2,
  );
  assert.equal(
    result.errors.filter((issue) => issue.code === 'record.identity.required')
      .length,
    2,
  );
});

test('distinguishes hard unresolved references from existing soft references', () => {
  const result = validateDatabase(
    snapshot({
      abilities: [
        {
          name: 'ability-one',
          statuses: [{ statusEffect: 'missing-status' }],
        },
      ],
      items: [
        {
          name: 'PotionHealing',
          useAbility: { abilityName: 'HEAL_SELF' },
        },
      ],
      maps: [
        map({
          layers: [-1, 0],
          tiles: { '-1': [0, 0], 0: [0, 0] },
          characters: [{ l: 0, i: 0, name: 'exampleTownsperson_2' }],
        }),
      ],
    }),
  );

  assert.equal(result.valid, false);
  assert.ok(
    result.errors.some(
      (issue) =>
        issue.code === 'reference.unresolved' &&
        issue.path.endsWith('.statusEffect'),
    ),
  );
  assert.ok(
    result.warnings.some(
      (issue) =>
        issue.code === 'reference.unresolved' &&
        issue.message.includes('HEAL_SELF'),
    ),
  );
  assert.ok(
    result.warnings.some((issue) =>
      issue.message.includes('exampleTownsperson_2'),
    ),
  );
  assert.ok(
    result.warnings.some((issue) => issue.code === 'map.layer.negative'),
  );
});

test('validates status effects against the runtime loader contract', () => {
  const result = validateDatabase(
    snapshot({
      statusEffects: [
        {
          name: 'BROKEN_STATUS',
          description: 'Has a condition the C++ loader cannot parse.',
          baseDuration: 1,
          actions: [
            {
              statusActionTargetType: 'STATUS_ACTION_TARGET_SELF',
              abilityName: '',
              events: [
                {
                  type: 'STATUS_EVENT_ON_APPLIED',
                  condition: 'CONDITION_IS_EVEN_ROUND',
                },
              ],
            },
          ],
        },
      ],
    }),
  );

  assert.equal(result.valid, false);
  assert.ok(
    result.errors.some(
      (issue) =>
        issue.code === 'statusEffect.schema' &&
        issue.path === 'statusEffects[0].actions[0].events[0].condition' &&
        issue.message.includes('unsupported value'),
    ),
  );
});

test('validates map dense arrays and map-grid matrix dimensions', () => {
  const result = validateDatabase(
    snapshot({
      maps: [map({ width: 2, height: 2, tiles: { 0: [0, 0] } })],
      mapGrids: [
        {
          name: 'grid-one',
          gridWidth: 2,
          gridHeight: 2,
          mapWidth: 2,
          mapHeight: 2,
          cells: [['map-one'], ['map-one', 42]],
        },
      ],
    }),
  );

  assert.ok(result.errors.some((issue) => issue.code === 'map.tiles.length'));
  assert.ok(
    result.errors.some((issue) => issue.code === 'mapGrid.cells.width'),
  );
  assert.ok(
    result.errors.some((issue) => issue.code === 'mapGrid.cells.string'),
  );
});

test('event node ids are unique and graph targets resolve inside their event', () => {
  const result = validateDatabase(
    snapshot({
      specialEvents: [
        {
          id: 'event-one',
          children: [
            { id: 'start', eventChildType: 'EXEC', next: 'missing' },
            { id: 'start', eventChildType: 'END' },
          ],
        },
      ],
    }),
  );

  assert.ok(
    result.errors.some((issue) => issue.code === 'event.node.id.duplicate'),
  );
  assert.ok(
    result.errors.some((issue) => issue.code === 'event.target.missing'),
  );
});

test('reference index supports inbound, outbound, and unresolved queries', () => {
  const database = snapshot({
    abilities: [{ name: 'ability-one' }],
    items: [
      {
        name: 'item-one',
        useAbility: { abilityName: 'ability-one' },
        useSpecialEvent: 'missing-event',
      },
    ],
    maps: [
      map({
        markers: [{ l: 0, i: 0, name: 'arrival' }],
        travelTriggers: [
          {
            l: 0,
            i: 0,
            destinationMapName: 'map-one',
            destinationMarkerName: 'arrival',
          },
        ],
      }),
    ],
  });
  const index = buildReferenceIndex(database);

  assert.equal(index.from('items', 'item-one').length, 2);
  assert.equal(index.to({ kind: 'abilities', id: 'ability-one' }).length, 1);
  assert.equal(
    index.to({ kind: 'mapMarker', id: 'arrival', scope: 'map-one' }).length,
    1,
  );
  assert.deepEqual(
    index.unresolved().map((reference) => reference.target.id),
    ['missing-event'],
  );
});
