import type { AssetId } from '../database/assetRegistry.js';
import type { DatabaseSnapshot } from '../database/types.js';
import type {
  DatabaseReference,
  ReferenceIntegrity,
  ReferenceTarget,
  ReferenceTargetQuery,
} from './types.js';

const IDENTITY_KEYS: Record<AssetId, 'id' | 'name'> = {
  statusEffects: 'name',
  abilities: 'name',
  items: 'name',
  spells: 'name',
  characters: 'name',
  maps: 'name',
  mapGrids: 'name',
  tilesets: 'name',
  specialEvents: 'id',
};

function isObject(value: unknown): value is Record<string, unknown> {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

function objects(value: unknown): Record<string, unknown>[] {
  return Array.isArray(value) ? value.filter(isObject) : [];
}

function text(value: unknown): string | null {
  return typeof value === 'string' && value.trim() ? value.trim() : null;
}

function recordId(
  assetId: AssetId,
  record: Record<string, unknown>,
  index: number,
) {
  return text(record[IDENTITY_KEYS[assetId]]) ?? `#${index}`;
}

function createTargetResolver(snapshot: DatabaseSnapshot) {
  const ids = new Map<AssetId, Set<string>>();
  for (const assetId of Object.keys(IDENTITY_KEYS) as AssetId[]) {
    const identityKey = IDENTITY_KEYS[assetId];
    ids.set(
      assetId,
      new Set(
        objects(snapshot[assetId])
          .map((record) => text(record[identityKey]))
          .filter((id): id is string => id !== null),
      ),
    );
  }

  const markersByMap = new Map<string, Set<string>>();
  for (const map of objects(snapshot.maps)) {
    const mapName = text(map.name);
    if (!mapName) {
      continue;
    }
    const markers = new Set(
      objects(map.markers)
        .map((marker) => text(marker.name))
        .filter((id): id is string => id !== null),
    );
    markersByMap.set(mapName, markers);
  }

  const nodesByEvent = new Map<string, Set<string>>();
  for (const event of objects(snapshot.specialEvents)) {
    const eventId = text(event.id);
    if (!eventId) {
      continue;
    }
    nodesByEvent.set(
      eventId,
      new Set(
        objects(event.children)
          .map((node) => text(node.id))
          .filter((id): id is string => id !== null),
      ),
    );
  }

  return (target: ReferenceTarget): boolean => {
    if (target.kind === 'mapOrMapGrid') {
      return Boolean(
        ids.get('maps')?.has(target.id) || ids.get('mapGrids')?.has(target.id),
      );
    }
    if (target.kind === 'mapMarker') {
      return Boolean(
        target.scope && markersByMap.get(target.scope)?.has(target.id),
      );
    }
    if (target.kind === 'eventNode') {
      return Boolean(
        target.scope && nodesByEvent.get(target.scope)?.has(target.id),
      );
    }
    return ids.get(target.kind)?.has(target.id) ?? false;
  };
}

function targetMatches(
  target: ReferenceTarget,
  query: ReferenceTargetQuery,
): boolean {
  if (target.id !== query.id || target.kind !== query.kind) {
    return false;
  }
  return query.scope === undefined || target.scope === query.scope;
}

/** Immutable query facade over all known structured database references. */
export class ReferenceIndex {
  readonly #references: readonly DatabaseReference[];

  constructor(references: DatabaseReference[]) {
    this.#references = Object.freeze(
      references.map((reference) =>
        Object.freeze({
          ...reference,
          source: Object.freeze({ ...reference.source }),
          target: Object.freeze({ ...reference.target }),
        }),
      ),
    );
  }

  all(): readonly DatabaseReference[] {
    return this.#references;
  }

  from(assetId: AssetId, recordId?: string): readonly DatabaseReference[] {
    return this.#references.filter(
      (reference) =>
        reference.source.assetId === assetId &&
        (recordId === undefined || reference.source.recordId === recordId),
    );
  }

  to(target: ReferenceTargetQuery): readonly DatabaseReference[] {
    return this.#references.filter((reference) =>
      targetMatches(reference.target, target),
    );
  }

  unresolved(): readonly DatabaseReference[] {
    return this.#references.filter((reference) => !reference.resolved);
  }
}

interface AddReferenceArgs {
  sourceAssetId: AssetId;
  sourceRecordId: string;
  path: string;
  relation: string;
  target: ReferenceTarget;
  integrity: ReferenceIntegrity;
}

export function buildReferenceIndex(
  snapshot: DatabaseSnapshot,
): ReferenceIndex {
  const references: DatabaseReference[] = [];
  const resolves = createTargetResolver(snapshot);
  const add = (args: AddReferenceArgs) => {
    references.push({
      relation: args.relation,
      source: {
        assetId: args.sourceAssetId,
        recordId: args.sourceRecordId,
        path: args.path,
      },
      target: args.target,
      integrity: args.integrity,
      resolved: resolves(args.target),
    });
  };
  const addText = (
    args: Omit<AddReferenceArgs, 'target'> & {
      targetKind: ReferenceTarget['kind'];
      value: unknown;
      scope?: string;
    },
  ) => {
    const id = text(args.value);
    if (!id) {
      return;
    }
    add({
      ...args,
      target: { kind: args.targetKind, id, scope: args.scope },
    });
  };

  objects(snapshot.abilities).forEach((ability, abilityIndex) => {
    const sourceRecordId = recordId('abilities', ability, abilityIndex);
    objects(ability.statuses).forEach((status, statusIndex) => {
      addText({
        sourceAssetId: 'abilities',
        sourceRecordId,
        path: `abilities[${abilityIndex}].statuses[${statusIndex}].statusEffect`,
        relation: 'ability.statusEffect',
        targetKind: 'statusEffects',
        value: status.statusEffect,
        integrity: 'hard',
      });
    });
  });

  objects(snapshot.statusEffects).forEach((status, statusIndex) => {
    const sourceRecordId = recordId('statusEffects', status, statusIndex);
    objects(status.actions).forEach((action, actionIndex) => {
      addText({
        sourceAssetId: 'statusEffects',
        sourceRecordId,
        path: `statusEffects[${statusIndex}].actions[${actionIndex}].abilityName`,
        relation: 'statusEffect.actionAbility',
        targetKind: 'abilities',
        value: action.abilityName,
        integrity: 'hard',
      });
    });
  });

  objects(snapshot.items).forEach((item, itemIndex) => {
    const sourceRecordId = recordId('items', item, itemIndex);
    if (Array.isArray(item.statusEffects)) {
      item.statusEffects.forEach((status, statusIndex) => {
        addText({
          sourceAssetId: 'items',
          sourceRecordId,
          path: `items[${itemIndex}].statusEffects[${statusIndex}]`,
          relation: 'item.statusEffect',
          targetKind: 'statusEffects',
          value: isObject(status) ? status.name : status,
          integrity: 'hard',
        });
      });
    }
    const abilityFields: Array<[unknown, string, string]> = [
      [
        isObject(item.weapon) ? item.weapon.abilityName : undefined,
        'weapon',
        'item.weaponAbility',
      ],
      [
        isObject(item.useAbility) ? item.useAbility.abilityName : undefined,
        'useAbility',
        'item.useAbility',
      ],
    ];
    for (const [value, field, relation] of abilityFields) {
      addText({
        sourceAssetId: 'items',
        sourceRecordId,
        path: `items[${itemIndex}].${field}.abilityName`,
        relation,
        targetKind: 'abilities',
        value,
        integrity: 'soft',
      });
    }
    addText({
      sourceAssetId: 'items',
      sourceRecordId,
      path: `items[${itemIndex}].useSpecialEvent`,
      relation: 'item.useSpecialEvent',
      targetKind: 'specialEvents',
      value: item.useSpecialEvent,
      integrity: 'soft',
    });
  });

  objects(snapshot.spells).forEach((spell, spellIndex) => {
    addText({
      sourceAssetId: 'spells',
      sourceRecordId: recordId('spells', spell, spellIndex),
      path: `spells[${spellIndex}].abilityName`,
      relation: 'spell.ability',
      targetKind: 'abilities',
      value: spell.abilityName,
      integrity: 'hard',
    });
  });

  objects(snapshot.characters).forEach((character, characterIndex) => {
    const sourceRecordId = recordId('characters', character, characterIndex);
    addText({
      sourceAssetId: 'characters',
      sourceRecordId,
      path: `characters[${characterIndex}].talk.talkName`,
      relation: 'character.talkEvent',
      targetKind: 'specialEvents',
      value: isObject(character.talk) ? character.talk.talkName : undefined,
      integrity: 'soft',
    });
    objects(character.statuses).forEach((status, statusIndex) => {
      addText({
        sourceAssetId: 'characters',
        sourceRecordId,
        path: `characters[${characterIndex}].statuses[${statusIndex}].status`,
        relation: 'character.statusEffect',
        targetKind: 'statusEffects',
        value: status.status,
        integrity: 'soft',
      });
    });
    for (const field of [
      'startingKnownSpells',
      'startingReadySpells',
    ] as const) {
      const values = character[field];
      if (!Array.isArray(values)) {
        continue;
      }
      values.forEach((value, valueIndex) => {
        addText({
          sourceAssetId: 'characters',
          sourceRecordId,
          path: `characters[${characterIndex}].${field}[${valueIndex}]`,
          relation: `character.${field}`,
          targetKind: 'spells',
          value,
          integrity: 'soft',
        });
      });
    }
  });

  objects(snapshot.maps).forEach((map, mapIndex) => {
    const sourceRecordId = recordId('maps', map, mapIndex);
    if (Array.isArray(map.tilesets)) {
      map.tilesets.forEach((value, tilesetIndex) => {
        if (!text(value)) {
          return;
        }
        addText({
          sourceAssetId: 'maps',
          sourceRecordId,
          path: `maps[${mapIndex}].tilesets[${tilesetIndex}]`,
          relation: 'map.tileset',
          targetKind: 'tilesets',
          value,
          integrity: 'soft',
        });
      });
    }
    const placementLists: Array<{
      field: string;
      targetKind: AssetId;
      relation: string;
      targetField: string;
    }> = [
      {
        field: 'characters',
        targetKind: 'characters',
        relation: 'map.character',
        targetField: 'name',
      },
      {
        field: 'items',
        targetKind: 'items',
        relation: 'map.item',
        targetField: 'name',
      },
      {
        field: 'eventTriggers',
        targetKind: 'specialEvents',
        relation: 'map.eventTrigger',
        targetField: 'eventId',
      },
    ];
    for (const placementList of placementLists) {
      objects(map[placementList.field]).forEach((placement, placementIndex) => {
        addText({
          sourceAssetId: 'maps',
          sourceRecordId,
          path: `maps[${mapIndex}].${placementList.field}[${placementIndex}].${placementList.targetField}`,
          relation: placementList.relation,
          targetKind: placementList.targetKind,
          value: placement[placementList.targetField],
          integrity: 'soft',
        });
      });
    }
    objects(map.travelTriggers).forEach((trigger, triggerIndex) => {
      const destination = text(trigger.destinationMapName);
      addText({
        sourceAssetId: 'maps',
        sourceRecordId,
        path: `maps[${mapIndex}].travelTriggers[${triggerIndex}].destinationMapName`,
        relation: 'map.travelDestination',
        targetKind: 'mapOrMapGrid',
        value: destination,
        integrity: 'soft',
      });
      addText({
        sourceAssetId: 'maps',
        sourceRecordId,
        path: `maps[${mapIndex}].travelTriggers[${triggerIndex}].destinationMarkerName`,
        relation: 'map.travelMarker',
        targetKind: 'mapMarker',
        value: trigger.destinationMarkerName,
        scope: destination ?? undefined,
        integrity: 'soft',
      });
    });
  });

  objects(snapshot.mapGrids).forEach((grid, gridIndex) => {
    const sourceRecordId = recordId('mapGrids', grid, gridIndex);
    if (!Array.isArray(grid.cells)) {
      return;
    }
    grid.cells.forEach((row, rowIndex) => {
      if (!Array.isArray(row)) {
        return;
      }
      row.forEach((value, columnIndex) => {
        addText({
          sourceAssetId: 'mapGrids',
          sourceRecordId,
          path: `mapGrids[${gridIndex}].cells[${rowIndex}][${columnIndex}]`,
          relation: 'mapGrid.map',
          targetKind: 'maps',
          value,
          integrity: 'soft',
        });
      });
    });
  });

  objects(snapshot.specialEvents).forEach((event, eventIndex) => {
    const sourceRecordId = recordId('specialEvents', event, eventIndex);
    if (Array.isArray(event.vars)) {
      event.vars.forEach((variable, variableIndex) => {
        if (!isObject(variable)) {
          return;
        }
        addText({
          sourceAssetId: 'specialEvents',
          sourceRecordId,
          path: `specialEvents[${eventIndex}].vars[${variableIndex}].importFrom`,
          relation: 'specialEvent.import',
          targetKind: 'specialEvents',
          value: variable.importFrom,
          integrity: 'soft',
        });
      });
    } else if (isObject(event.vars)) {
      Object.entries(event.vars).forEach(([variableId, variable]) => {
        if (!isObject(variable)) {
          return;
        }
        addText({
          sourceAssetId: 'specialEvents',
          sourceRecordId,
          path: `specialEvents[${eventIndex}].vars.${variableId}.importFrom`,
          relation: 'specialEvent.import',
          targetKind: 'specialEvents',
          value: variable.importFrom,
          integrity: 'soft',
        });
      });
    }

    objects(event.children).forEach((node, nodeIndex) => {
      const nodeFields: Array<[unknown, string]> = [
        [node.next, 'next'],
        [node.defaultNext, 'defaultNext'],
      ];
      for (const [value, field] of nodeFields) {
        addText({
          sourceAssetId: 'specialEvents',
          sourceRecordId,
          path: `specialEvents[${eventIndex}].children[${nodeIndex}].${field}`,
          relation: 'specialEvent.nodeTarget',
          targetKind: 'eventNode',
          value,
          scope: sourceRecordId,
          integrity: 'structural',
        });
      }
      const nestedTargets: Array<[string, string]> = [
        ['choices', 'next'],
        ['cases', 'next'],
      ];
      for (const [listField, targetField] of nestedTargets) {
        const list = node[listField];
        if (!Array.isArray(list)) {
          continue;
        }
        list.forEach((entry, entryIndex) => {
          if (!isObject(entry)) {
            return;
          }
          addText({
            sourceAssetId: 'specialEvents',
            sourceRecordId,
            path: `specialEvents[${eventIndex}].children[${nodeIndex}].${listField}[${entryIndex}].${targetField}`,
            relation: 'specialEvent.nodeTarget',
            targetKind: 'eventNode',
            value: entry[targetField],
            scope: sourceRecordId,
            integrity: 'structural',
          });
        });
      }
    });
  });

  return new ReferenceIndex(references);
}
