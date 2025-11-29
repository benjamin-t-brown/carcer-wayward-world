import { GameEventChildSwitch, SwitchCase } from '../../types/assets';
import { drawRect, drawText } from '../../utils/draw';
import { renderCloseButton } from './closeButton';
import { RenderNodeArgs } from '../nodeHelpers';

const NODE_COLOR = '#808080';
const BORDER_COLOR = '#aaa';
const BORDER_HOVER_COLOR = '#fff';
const TEXT_COLOR = '#FFF';
const FONT_FAMILY = 'arial';
const NODE_WIDTH = 300;
const NODE_TITLE_HEIGHT = 20;
const BORDER_WIDTH = 2;
const FONT_SIZE = 12;
const PADDING = 6;
const LINE_SPACING = 4;
const LINE_HEIGHT = 14;

export const getSwitchNodeDimensions = (node: GameEventChildSwitch) => {
  return [NODE_WIDTH, node.h];
};

export const getSwitchNodeChildren = (node: GameEventChildSwitch): string[] => {
  const children: string[] = [];
  // Add all case next IDs
  node.cases.forEach((caseItem) => {
    if (caseItem.next) {
      children.push(caseItem.next);
    }
  });
  // Add default next if it exists
  if (node.defaultNext) {
    children.push(node.defaultNext);
  }
  return children;
};

export const renderSwitchNode = (
  node: GameEventChildSwitch,
  x: number,
  y: number,
  scale: number,
  ctx: CanvasRenderingContext2D,
  args: RenderNodeArgs
) => {
  // Calculate height based on number of cases
  const caseCount = node.cases.length;
  const caseHeight = LINE_HEIGHT;
  const totalCaseHeight = caseCount * caseHeight;
  const minHeight = NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
  node.h = Math.max(minHeight, totalCaseHeight + NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2);

  const nodeX = x * scale;
  const nodeY = y * scale;
  const nodeWidth = NODE_WIDTH * scale;
  const nodeHeight = node.h * scale;

  // Draw border - red if selected, white if hovered, light green if child of hovered, light red if parent of hovered, gray otherwise
  const borderColor = args.isSelected
    ? '#ff0000'
    : args.isHovered
    ? BORDER_HOVER_COLOR
    : args.isChildOfHovered
    ? '#90EE90' // Light green
    : args.isParentOfHovered
    ? '#FFB6C1' // Light red
    : BORDER_COLOR;
  const borderWidth = args.isSelected ? 3 : BORDER_WIDTH;
  
  drawRect(
    nodeX,
    nodeY,
    nodeWidth,
    nodeHeight,
    borderColor,
    false,
    ctx
  );
  
  // Draw additional border for selected nodes
  if (args.isSelected) {
    drawRect(
      nodeX + borderWidth,
      nodeY + borderWidth,
      nodeWidth - borderWidth * 2,
      nodeHeight - borderWidth * 2,
      borderColor,
      false,
      ctx
    );
  }
  drawRect(
    nodeX + BORDER_WIDTH,
    nodeY + BORDER_WIDTH,
    nodeWidth - BORDER_WIDTH * 2,
    nodeHeight - BORDER_WIDTH * 2,
    NODE_COLOR,
    false,
    ctx
  );

  renderCloseButton(node, scale, ctx, {
    isHovered: args.isCloseButtonHovered || false,
    isActive: false,
  });

  // title
  ctx.save();
  ctx.translate(nodeX, nodeY);
  ctx.scale(scale, scale);
  ctx.translate(PADDING + BORDER_WIDTH, PADDING + BORDER_WIDTH);
  drawText(
    node.id,
    0,
    0,
    {
      color: '#ddd',
      size: FONT_SIZE,
      font: FONT_FAMILY,
      strokeColor: '',
      align: 'left',
      baseline: 'top',
    },
    ctx
  );
  ctx.restore();

  // Render each case's conditionStr
  for (let i = 0; i < node.cases.length; i++) {
    const caseItem = node.cases[i];
    ctx.save();
    ctx.translate(nodeX, nodeY);
    ctx.scale(scale, scale);
    ctx.translate(
      PADDING + BORDER_WIDTH,
      PADDING + BORDER_WIDTH + NODE_TITLE_HEIGHT + i * LINE_HEIGHT
    );
    drawText(
      caseItem.conditionStr || '',
      0,
      0,
      {
        color: TEXT_COLOR,
        size: FONT_SIZE,
        font: FONT_FAMILY,
        strokeColor: '',
        align: 'left',
        baseline: 'top',
      },
      ctx
    );
    ctx.restore();
  }
};

