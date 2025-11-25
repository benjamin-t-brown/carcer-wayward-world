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

export const calculateNodeHeight = (
  lines: string[],
  fontSize: number,
  fontFamily: string,
  padding: number,
  borderWidth: number,
  lineHeight: number,
  lineSpacing: number,
  ctx: CanvasRenderingContext2D
): number => {
  if (lines.length === 0) {
    // Minimum height for empty node
    return padding * 2 + borderWidth * 2;
  }

  // Set font to measure text
  ctx.font = `${fontSize}px ${fontFamily}`;
  ctx.textAlign = 'left';
  ctx.textBaseline = 'top';

  let lineCount = 0;

  for (let i = 0; i < lines.length; i++) {
    lineCount++;
  }

  // Calculate total height: padding + (line count * line height)
  const totalHeight =
    padding * 2 +
    borderWidth * 2 +
    lineCount * lineHeight -
    (lineCount > 0 ? lineSpacing : 0);

  return Math.max(totalHeight, padding * 2 + borderWidth * 2); // Minimum height
};

export const renderCloseButton = (
  nodeX: number,
  nodeY: number,
  nodeWidth: number,
  borderWidth: number,
  scale: number,
  ctx: CanvasRenderingContext2D
) => {
  const btnSize = 15;
  ctx.save();
  ctx.translate(nodeX, nodeY);
  ctx.translate(nodeWidth - btnSize * scale - borderWidth * 2, borderWidth * 2);
  ctx.scale(scale, scale);
  drawRect(0, 0, btnSize, btnSize, '#800', false, ctx);
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
