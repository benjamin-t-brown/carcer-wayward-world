import { CharacterTemplate, GameEvent } from '../types/assets';

export function isGameEventIconRequired(eventType: GameEvent['eventType']): boolean {
  return eventType !== 'TALK';
}

/** TALK events prefer the first linked character portrait, then the event icon. */
export function resolveTalkEventIcon(
  gameEvent: Pick<GameEvent, 'id' | 'eventType' | 'icon'>,
  characters: CharacterTemplate[],
): string {
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
