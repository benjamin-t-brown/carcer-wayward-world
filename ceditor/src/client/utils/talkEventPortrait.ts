import { CharacterTemplate, GameEvent } from '../types/assets';

export function isGameEventIconRequired(eventType: GameEvent['eventType']): boolean {
  return eventType !== 'TALK';
}

export function getTalkPortName(
  storage: Record<string, unknown> | undefined,
): string {
  const tmp = storage?.tmp;
  if (!tmp || typeof tmp !== 'object') {
    return '';
  }
  const talk = (tmp as { talk?: unknown }).talk;
  if (!talk || typeof talk !== 'object') {
    return '';
  }
  const port = (talk as { port?: unknown }).port;
  return typeof port === 'string' ? port.trim() : '';
}

function portraitForCharacter(
  characters: CharacterTemplate[],
  characterName: string,
): string {
  const character = characters.find((entry) => entry.name === characterName);
  return character?.talk?.portraitName?.trim() ?? '';
}

/** TALK events prefer the first linked character portrait, then the event icon.
 * SET_PORT(characterName) swaps in that character's portrait for this conversation.
 * If the name is not a character, it is treated as a portrait sprite. */
export function resolveTalkEventIcon(
  gameEvent: Pick<GameEvent, 'id' | 'eventType' | 'icon'>,
  characters: CharacterTemplate[],
  auxName = '',
): string {
  const trimmedAux = auxName.trim();
  if (trimmedAux) {
    const auxPortrait = portraitForCharacter(characters, trimmedAux);
    if (auxPortrait) {
      return auxPortrait;
    }
    const auxIsCharacter = characters.some((entry) => entry.name === trimmedAux);
    if (!auxIsCharacter) {
      return trimmedAux;
    }
  }

  if (gameEvent.eventType === 'TALK') {
    for (const character of characters) {
      const portrait = character.talk?.portraitName?.trim();
      if (character.talk?.talkName === gameEvent.id && portrait) {
        return portrait;
      }
    }
  }
  return gameEvent.icon?.trim() ?? '';
}
