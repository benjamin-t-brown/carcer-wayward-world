import type {
  CarcerMapTemplate,
  CharacterTemplate,
  GameEvent,
  ItemTemplate,
} from '../types/assets';
import {
  findGameEventReferences,
  renameGameEventIdInCharacters,
  renameGameEventIdInGameEventImports,
  renameGameEventIdInItems,
  renameGameEventIdInMaps,
} from './gameEventReferences';
import { trimStrings } from './jsonUtils';

export type RenamedReferenceCollection = 'maps' | 'characters' | 'items';

export interface AtomicGameEventRename {
  gameEvents: GameEvent[];
  maps?: CarcerMapTemplate[];
  characters?: CharacterTemplate[];
  items?: ItemTemplate[];
  renamedReferenceCollections: RenamedReferenceCollection[];
}

interface PrepareAtomicGameEventRenameOptions {
  gameEvents: GameEvent[];
  maps: CarcerMapTemplate[];
  characters: CharacterTemplate[];
  items: ItemTemplate[];
  previousEventId: string;
  updatedGameEvent: GameEvent;
}

/** Prepare every collection affected by an event rename without mutating input. */
export function prepareAtomicGameEventRename({
  gameEvents,
  maps,
  characters,
  items,
  previousEventId,
  updatedGameEvent,
}: PrepareAtomicGameEventRenameOptions): AtomicGameEventRename {
  const eventIndex = gameEvents.findIndex(
    (gameEvent) => gameEvent.id === previousEventId,
  );
  if (eventIndex === -1) {
    throw new Error(`Game event "${previousEventId}" no longer exists`);
  }

  const nextGameEvents = [...gameEvents];
  nextGameEvents[eventIndex] = structuredClone(updatedGameEvent);
  const references = findGameEventReferences(
    maps,
    characters,
    items,
    nextGameEvents,
    previousEventId,
  );
  const renamedReferenceCollections: RenamedReferenceCollection[] = [];
  const result: AtomicGameEventRename = {
    gameEvents: prepareGameEventsForSave(
      renameGameEventIdInGameEventImports(
        nextGameEvents,
        previousEventId,
        updatedGameEvent.id,
      ),
    ),
    renamedReferenceCollections,
  };

  if (references.tiles.length > 0) {
    result.maps = trimStrings(
      renameGameEventIdInMaps(maps, previousEventId, updatedGameEvent.id),
    );
    renamedReferenceCollections.push('maps');
  }
  if (references.characters.length > 0) {
    result.characters = trimStrings(
      renameGameEventIdInCharacters(
        characters,
        previousEventId,
        updatedGameEvent.id,
      ),
    );
    renamedReferenceCollections.push('characters');
  }
  if (references.items.length > 0) {
    result.items = trimStrings(
      renameGameEventIdInItems(items, previousEventId, updatedGameEvent.id),
    );
    renamedReferenceCollections.push('items');
  }

  return result;
}

export function prepareGameEventsForSave(gameEvents: GameEvent[]): GameEvent[] {
  return trimStrings(gameEvents).sort((a, b) => a.id.localeCompare(b.id));
}
