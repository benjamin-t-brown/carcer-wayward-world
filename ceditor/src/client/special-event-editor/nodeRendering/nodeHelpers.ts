import {
  GameEvent,
  GameEventChildExec,
  GameEventChildType,
  SENode,
} from '../../types/assets';
import { drawRect, drawText } from '../../utils/draw';
import { getExecNodeDimensions } from './renderExecNode';

export const breakTextIntoLines = (
  text: string,
  availableWidth: number,
  fontSize: number,
  fontFamily: string,
  ctx: CanvasRenderingContext2D
): string[] => {
  const lines: string[] = [];

  ctx.font = `${fontSize}px ${fontFamily}`;
  ctx.textAlign = 'left';
  ctx.textBaseline = 'top';

  // Split by explicit newlines first
  const paragraphs = text.split('\n');

  for (const paragraph of paragraphs) {
    if (paragraph.trim() === '') {
      lines.push('');
      continue;
    }

    // Word wrap within the paragraph
    const words = paragraph.split(' ');
    let currentLine = '';

    for (const word of words) {
      const testLine = currentLine ? `${currentLine} ${word}` : word;
      const metrics = ctx.measureText(testLine);

      if (metrics.width > availableWidth && currentLine) {
        // Current line is full, start a new one
        lines.push(currentLine);
        currentLine = word;
      } else {
        currentLine = testLine;
      }
    }

    if (currentLine) {
      lines.push(currentLine);
    }
  }

  return lines;
};

export const calculateHeightFromText = (
  lines: string[],
  fontSize: number,
  fontFamily: string,
  lineHeight: number,
  lineSpacing: number,
  ctx: CanvasRenderingContext2D
): number => {
  if (lines.length === 0) {
    return 0;
  }

  // Set font to measure text
  ctx.font = `${fontSize}px ${fontFamily}`;
  ctx.textAlign = 'left';
  ctx.textBaseline = 'top';

  let lineCount = 0;

  for (let i = 0; i < lines.length; i++) {
    lineCount++;
  }

  const totalHeight =
    lineCount * lineHeight - (lineCount > 0 ? lineSpacing : 0);

  return Math.max(totalHeight, 0);
};



export const getCloseButtonBounds = (
  nodeX: number,
  nodeY: number,
  nodeWidth: number,
  borderWidth: number,
  scale: number
) => {
  const btnSize = 15;
  return {
    x: nodeX + nodeWidth - btnSize * scale - borderWidth * 2,
    y: nodeY + borderWidth * 2,
  };
};

export const getNodeBounds = (child: SENode) => {
  let nodeWidth = 0;
  if (child.eventChildType === GameEventChildType.EXEC) {
    const [NODE_WIDTH] = getExecNodeDimensions(child as GameEventChildExec);
    nodeWidth = NODE_WIDTH;
  }
  return [nodeWidth, child.h];
};

interface AccessibleVariable {
  key: string;
  value: string;
  source: string;
}

export const getVarsFromNode = (
  gameEvent: GameEvent,
  gameEvents: GameEvent[]
) => {
  const usedNodes: string[] = [];

  const innerHelper = (gameEvent: GameEvent) => {
    const vars: AccessibleVariable[] = [];
    // infinite loop protection
    if (usedNodes.includes(gameEvent.id)) {
      return [];
    }
    usedNodes.push(gameEvent.id);

    for (const variable of gameEvent.vars) {
      if (variable.importFrom === '') {
        vars.push({
          key: variable.key,
          value: variable.value,
          source: gameEvent.id,
        });
      }
    }

    for (const variable of gameEvent.vars) {
      if (variable.importFrom !== '') {
        const sourceEvent = gameEvents.find(
          (event) => event.id === variable.importFrom
        );
        if (sourceEvent) {
          vars.push(...innerHelper(sourceEvent));
        }
      }
    }
    return vars;
  };

  return innerHelper(gameEvent);
};
