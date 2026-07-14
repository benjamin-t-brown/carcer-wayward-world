import {
  CarcerMapTemplate,
  CharacterTemplate,
  GameEvent,
  ItemTemplate,
} from '../types/assets';

export interface GameEventTileReference {
  mapName: string;
  layer: number;
  x: number;
  y: number;
}

export interface GameEventCharacterReference {
  characterName: string;
}

export interface GameEventItemReference {
  itemName: string;
}

export interface GameEventImportReference {
  eventId: string;
  eventTitle: string;
}

export interface GameEventReferences {
  tiles: GameEventTileReference[];
  characters: GameEventCharacterReference[];
  items: GameEventItemReference[];
  eventImports: GameEventImportReference[];
}

export function findGameEventReferences(
  maps: CarcerMapTemplate[],
  characters: CharacterTemplate[],
  items: ItemTemplate[],
  gameEvents: GameEvent[],
  eventId: string,
): GameEventReferences {
  const tiles: GameEventTileReference[] = [];
  for (const map of maps) {
    for (const trigger of map.eventTriggers ?? []) {
      if (trigger.eventId !== eventId) {
        continue;
      }
      tiles.push({
        mapName: map.name,
        layer: trigger.l,
        x: trigger.i % map.width,
        y: Math.floor(trigger.i / map.width),
      });
    }
  }

  const characterRefs: GameEventCharacterReference[] = [];
  for (const character of characters) {
    if (character.talk?.talkName === eventId) {
      characterRefs.push({ characterName: character.name });
    }
  }

  const itemRefs: GameEventItemReference[] = [];
  for (const item of items) {
    if (item.useSpecialEvent === eventId) {
      itemRefs.push({ itemName: item.name });
    }
  }

  const eventImports: GameEventImportReference[] = [];
  for (const event of gameEvents) {
    if (event.vars.some((variable) => variable.importFrom === eventId)) {
      eventImports.push({
        eventId: event.id,
        eventTitle: event.title,
      });
    }
  }

  return {
    tiles,
    characters: characterRefs,
    items: itemRefs,
    eventImports,
  };
}

export function gameEventReferencesTotal(
  references: GameEventReferences,
): number {
  return (
    references.tiles.length +
    references.characters.length +
    references.items.length +
    references.eventImports.length
  );
}

export function formatGameEventRenameWarning(
  oldId: string,
  newId: string,
  references: GameEventReferences,
): string {
  const lines = [
    `Renaming "${oldId}" to "${newId}" is used elsewhere. References must be updated or they will break.`,
    '',
  ];

  if (references.tiles.length > 0) {
    lines.push(
      `Tiles (${references.tiles.length}):`,
      ...references.tiles.map(
        (ref) =>
          `• ${ref.mapName} layer ${ref.layer} at (${ref.x}, ${ref.y})`,
      ),
      '',
    );
  }

  if (references.characters.length > 0) {
    lines.push(
      `Characters (${references.characters.length}):`,
      ...references.characters.map((ref) => `• ${ref.characterName}`),
      '',
    );
  }

  if (references.items.length > 0) {
    lines.push(
      `Items (${references.items.length}):`,
      ...references.items.map((ref) => `• ${ref.itemName}`),
      '',
    );
  }

  if (references.eventImports.length > 0) {
    lines.push(
      `Events importing variables (${references.eventImports.length}):`,
      ...references.eventImports.map(
        (ref) => `• ${ref.eventId}${ref.eventTitle ? ` (${ref.eventTitle})` : ''}`,
      ),
      '',
    );
  }

  lines.push('Rename and update references? Referenced asset files will be saved automatically.');
  return lines.join('\n');
}

export function renameGameEventIdInMaps(
  maps: CarcerMapTemplate[],
  oldId: string,
  newId: string,
): CarcerMapTemplate[] {
  return maps.map((map) => ({
    ...map,
    eventTriggers: (map.eventTriggers ?? []).map((trigger) =>
      trigger.eventId === oldId ? { ...trigger, eventId: newId } : trigger,
    ),
  }));
}

export function renameGameEventIdInCharacters(
  characters: CharacterTemplate[],
  oldId: string,
  newId: string,
): CharacterTemplate[] {
  return characters.map((character) => {
    if (character.talk?.talkName !== oldId) {
      return character;
    }
    return {
      ...character,
      talk: {
        ...character.talk,
        talkName: newId,
      },
    };
  });
}

export function renameGameEventIdInItems(
  items: ItemTemplate[],
  oldId: string,
  newId: string,
): ItemTemplate[] {
  return items.map((item) =>
    item.useSpecialEvent === oldId
      ? { ...item, useSpecialEvent: newId }
      : item,
  );
}

export function renameGameEventIdInGameEventImports(
  gameEvents: GameEvent[],
  oldId: string,
  newId: string,
): GameEvent[] {
  return gameEvents.map((event) => ({
    ...event,
    vars: event.vars.map((variable) =>
      variable.importFrom === oldId
        ? { ...variable, importFrom: newId }
        : variable,
    ),
  }));
}
