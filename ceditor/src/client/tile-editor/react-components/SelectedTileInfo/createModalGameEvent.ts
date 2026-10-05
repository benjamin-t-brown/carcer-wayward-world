import {
  GameEvent,
  GameEventChildType,
  SENode,
} from '../../../types/assets';
import { randomId } from '../../../utils/mathUtils';

export const DEFAULT_MODAL_NODE_TEXT = 'This is the default text.';

export function createModalGameEvent(id: string, text: string): GameEvent {
  const trimmed = id.trim();
  const textId = randomId();
  const endId = randomId();
  const textHeight = text.length > 80 || text.includes('\n') ? 88 : 50;

  return {
    id: trimmed,
    title: 'Event',
    eventType: 'MODAL',
    icon: 'special_event_icons_0',
    vars: [],
    children: [
      {
        id: 'root',
        x: 20,
        y: 20,
        eventChildType: GameEventChildType.EXEC,
        h: 50,
        p: '',
        execStr: '',
        next: textId,
        autoAdvance: true,
      },
      {
        id: textId,
        x: 248,
        y: 128,
        eventChildType: GameEventChildType.EXEC,
        h: textHeight,
        p: text,
        execStr: '',
        next: endId,
        autoAdvance: true,
      },
      {
        id: endId,
        x: 454,
        y: 249,
        eventChildType: GameEventChildType.END,
        h: 60,
        next: '',
      },
    ] as unknown as SENode[],
  };
}
