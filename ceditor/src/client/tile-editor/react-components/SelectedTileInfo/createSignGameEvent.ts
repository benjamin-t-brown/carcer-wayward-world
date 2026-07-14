import {
  GameEvent,
  GameEventChildType,
  SENode,
} from '../../../types/assets';
import { randomId } from '../../../utils/mathUtils';

export function createSignGameEvent(args: {
  id: string;
  title: string;
  contents: string;
}): GameEvent {
  const textId = randomId();
  const endId = randomId();
  const contents = args.contents.trim();
  const signText = `The sign says: "${contents}"`;
  const textHeight =
    contents.length > 80 || contents.includes('\n') ? 88 : 50;

  return {
    id: args.id.trim(),
    title: args.title.trim(),
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
        p: signText,
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
    ] as SENode[],
  };
}

/** Suggest `{mapPrefix}_sign_` from a map name like `alinea_outsideAlinea1`. */
export function suggestSignEventIdPrefix(mapName: string): string {
  const trimmed = mapName.trim();
  if (!trimmed) {
    return 'sign_';
  }
  const underscore = trimmed.indexOf('_');
  const prefix =
    underscore > 0 ? trimmed.slice(0, underscore) : trimmed;
  return `${prefix.toLowerCase()}_sign_`;
}
