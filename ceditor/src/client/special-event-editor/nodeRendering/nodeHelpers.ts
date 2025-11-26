import { drawRect, drawText } from '../../utils/draw';

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

export const renderCloseButton = (
  nodeX: number,
  nodeY: number,
  nodeWidth: number,
  borderWidth: number,
  scale: number,
  ctx: CanvasRenderingContext2D,
  args: {
    isHovered: boolean;
    isActive: boolean;
  }
) => {
  const btnSize = 15;
  ctx.save();
  ctx.translate(nodeX, nodeY);
  ctx.translate(nodeWidth - btnSize * scale - borderWidth * 2, borderWidth * 2);
  ctx.scale(scale, scale);
  drawRect(
    0,
    0,
    btnSize,
    btnSize,
    args.isActive ? '#500' : args.isHovered ? '#a00' : '#800',
    false,
    ctx
  );
  drawText(
    'X',
    4,
    2,
    {
      color: '#fff',
      size: 12,
      font: 'courier',
      strokeColor: '',
      align: 'left',
      baseline: 'top',
    },
    ctx
  );
  ctx.restore();
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
