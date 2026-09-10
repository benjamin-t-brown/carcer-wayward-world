import type {
  JsonArray,
  JsonObject,
  JsonValue,
} from '../../core/database/index.js';
import { parseSpecialEventCollection } from '../../core/domain/events/index.js';

export type EventReferenceKind =
  'map-trigger' | 'character-talk' | 'item-use' | 'event-import';

export interface EventReferenceLocation {
  readonly kind: EventReferenceKind;
  readonly path: string;
  readonly label: string;
  readonly recordIndex: number;
  readonly nestedIndex?: number;
}

export interface EventReferencePreview {
  readonly targetId: string;
  readonly eventIndex: number;
  readonly references: readonly EventReferenceLocation[];
}

export interface EventDatabaseRecords {
  readonly specialEvents: readonly JsonValue[];
  readonly maps: readonly JsonValue[];
  readonly characters: readonly JsonValue[];
  readonly items: readonly JsonValue[];
}

export interface EventLifecycleResult {
  readonly specialEvents: JsonArray;
  readonly maps: JsonArray;
  readonly characters: JsonArray;
  readonly items: JsonArray;
  readonly preview: EventReferencePreview;
}

export class EventLifecycleError extends Error {
  constructor(message: string) {
    super(message);
    this.name = 'EventLifecycleError';
  }
}

function object(value: JsonValue | undefined): JsonObject | undefined {
  return typeof value === 'object' && value !== null && !Array.isArray(value)
    ? value
    : undefined;
}

function text(value: JsonValue | undefined): string | undefined {
  return typeof value === 'string' ? value : undefined;
}

function eventRecordId(record: JsonValue | undefined, index: number): string {
  return text(object(record)?.id) ?? `event ${index + 1}`;
}

function namedRecord(record: JsonValue | undefined, index: number): string {
  const value = object(record);
  return text(value?.name) ?? text(value?.label) ?? `record ${index + 1}`;
}

function requireTarget(
  records: EventDatabaseRecords,
  targetIdValue: string,
): { id: string; index: number } {
  const id = targetIdValue.trim();
  if (!id) throw new EventLifecycleError('Event ID cannot be empty.');
  const matches = records.specialEvents
    .map((record, index) => ({ id: text(object(record)?.id), index }))
    .filter((candidate) => candidate.id === id);
  if (matches.length !== 1) {
    throw new EventLifecycleError(
      matches.length === 0
        ? `Event "${id}" was not found.`
        : `Event ID "${id}" is ambiguous.`,
    );
  }
  return { id, index: matches[0]!.index };
}

/** Enumerate all structured database references affected by an event rename or deletion. */
export function previewEventReferences(
  records: EventDatabaseRecords,
  targetId: string,
): EventReferencePreview {
  const target = requireTarget(records, targetId);
  const references: EventReferenceLocation[] = [];
  records.maps.forEach((rawMap, mapIndex) => {
    const map = object(rawMap);
    if (!map || !Array.isArray(map.eventTriggers)) return;
    map.eventTriggers.forEach((rawTrigger, triggerIndex) => {
      if (text(object(rawTrigger)?.eventId) !== target.id) return;
      references.push({
        kind: 'map-trigger',
        path: `maps[${mapIndex}].eventTriggers[${triggerIndex}].eventId`,
        label: `Map ${namedRecord(rawMap, mapIndex)} trigger ${triggerIndex + 1}`,
        recordIndex: mapIndex,
        nestedIndex: triggerIndex,
      });
    });
  });
  records.characters.forEach((rawCharacter, characterIndex) => {
    const character = object(rawCharacter);
    if (text(object(character?.talk)?.talkName) !== target.id) return;
    references.push({
      kind: 'character-talk',
      path: `characters[${characterIndex}].talk.talkName`,
      label: `Character ${namedRecord(rawCharacter, characterIndex)} talk event`,
      recordIndex: characterIndex,
    });
  });
  records.items.forEach((rawItem, itemIndex) => {
    if (text(object(rawItem)?.useSpecialEvent) !== target.id) return;
    references.push({
      kind: 'item-use',
      path: `items[${itemIndex}].useSpecialEvent`,
      label: `Item ${namedRecord(rawItem, itemIndex)} use event`,
      recordIndex: itemIndex,
    });
  });
  records.specialEvents.forEach((rawEvent, eventIndex) => {
    const event = object(rawEvent);
    if (!event || !Array.isArray(event.vars)) return;
    event.vars.forEach((rawVariable, variableIndex) => {
      if (text(object(rawVariable)?.importFrom) !== target.id) return;
      references.push({
        kind: 'event-import',
        path: `specialEvents[${eventIndex}].vars[${variableIndex}].importFrom`,
        label: `Event ${eventRecordId(rawEvent, eventIndex)} import ${variableIndex + 1}`,
        recordIndex: eventIndex,
        nestedIndex: variableIndex,
      });
    });
  });
  return { targetId: target.id, eventIndex: target.index, references };
}

function cloneRecords(
  records: EventDatabaseRecords,
): Omit<EventLifecycleResult, 'preview'> {
  return {
    specialEvents: parseSpecialEventCollection(
      records.specialEvents,
    ) as JsonArray,
    maps: structuredClone(records.maps) as JsonArray,
    characters: structuredClone(records.characters) as JsonArray,
    items: structuredClone(records.items) as JsonArray,
  };
}

function selected(
  reference: EventReferenceLocation,
  selectedPaths: ReadonlySet<string> | undefined,
): boolean {
  return selectedPaths === undefined || selectedPaths.has(reference.path);
}

export function renameEventAcrossDatabase(
  records: EventDatabaseRecords,
  targetId: string,
  nextIdValue: string,
  selectedPaths?: ReadonlySet<string>,
): EventLifecycleResult {
  const preview = previewEventReferences(records, targetId);
  const nextId = nextIdValue.trim();
  if (!nextId) throw new EventLifecycleError('Event ID cannot be empty.');
  if (
    records.specialEvents.some(
      (record, index) =>
        index !== preview.eventIndex && text(object(record)?.id) === nextId,
    )
  ) {
    throw new EventLifecycleError(`Event "${nextId}" already exists.`);
  }
  const result = cloneRecords(records);
  object(result.specialEvents[preview.eventIndex])!.id = nextId;
  for (const reference of preview.references) {
    if (!selected(reference, selectedPaths)) continue;
    if (reference.kind === 'map-trigger') {
      const map = object(result.maps[reference.recordIndex])!;
      object(
        (map.eventTriggers as JsonArray)[reference.nestedIndex!],
      )!.eventId = nextId;
    } else if (reference.kind === 'character-talk') {
      object(object(result.characters[reference.recordIndex])!.talk)!.talkName =
        nextId;
    } else if (reference.kind === 'item-use') {
      object(result.items[reference.recordIndex])!.useSpecialEvent = nextId;
    } else {
      const event = object(result.specialEvents[reference.recordIndex])!;
      object((event.vars as JsonArray)[reference.nestedIndex!])!.importFrom =
        nextId;
    }
  }
  return { ...result, preview };
}

/**
 * Delete one event and explicitly clean only the selected references. Map
 * triggers and import variable records are removed; optional character/item
 * links are cleared. Unselected references remain for validation to report.
 */
export function deleteEventAcrossDatabase(
  records: EventDatabaseRecords,
  targetId: string,
  selectedPaths?: ReadonlySet<string>,
): EventLifecycleResult {
  const preview = previewEventReferences(records, targetId);
  const result = cloneRecords(records);
  const references = preview.references.filter((reference) =>
    selected(reference, selectedPaths),
  );
  const mapTriggerPaths = new Set(
    references
      .filter(({ kind }) => kind === 'map-trigger')
      .map(({ path }) => path),
  );
  result.maps.forEach((rawMap, mapIndex) => {
    const map = object(rawMap);
    if (!map || !Array.isArray(map.eventTriggers)) return;
    map.eventTriggers = map.eventTriggers.filter(
      (_, triggerIndex) =>
        !mapTriggerPaths.has(
          `maps[${mapIndex}].eventTriggers[${triggerIndex}].eventId`,
        ),
    );
  });
  for (const reference of references) {
    if (reference.kind === 'character-talk') {
      object(object(result.characters[reference.recordIndex])!.talk)!.talkName =
        '';
    } else if (reference.kind === 'item-use') {
      delete object(result.items[reference.recordIndex])!.useSpecialEvent;
    }
  }
  const importPaths = new Set(
    references
      .filter(({ kind }) => kind === 'event-import')
      .map(({ path }) => path),
  );
  result.specialEvents.forEach((rawEvent, eventIndex) => {
    const event = object(rawEvent);
    if (!event || !Array.isArray(event.vars)) return;
    event.vars = event.vars.filter(
      (_, variableIndex) =>
        !importPaths.has(
          `specialEvents[${eventIndex}].vars[${variableIndex}].importFrom`,
        ),
    );
  });
  result.specialEvents.splice(preview.eventIndex, 1);
  return { ...result, preview };
}
