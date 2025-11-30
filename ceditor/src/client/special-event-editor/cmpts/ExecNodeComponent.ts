import { GameEventChildExec } from '../../types/assets';
import { drawText } from '../../utils/draw';
import { breakTextIntoLines, calculateHeightFromText } from '../nodeHelpers';
import { EditorNode, RenderNodeArgs } from '../EditorNode';
import { Connector } from './Connector';
import { EditorStateSE } from '../seEditorState';

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

export class EditorNodeExec extends EditorNode {
  p: string = '';
  execStr: string = '';

  constructor(seNode: GameEventChildExec, editorState: EditorStateSE) {
    super(seNode, editorState);

    this.bgColor = NODE_COLOR;
    this.borderColor = BORDER_COLOR;
    this.borderColorHovered = BORDER_HOVER_COLOR;
    this.padding = PADDING;
    this.borderSize = BORDER_WIDTH;

    this.p = seNode.p;
    this.execStr = seNode.execStr;
    this.exits.push(
      Connector.create(
        this.id,
        seNode.next,
        this.x + this.width,
        this.y + this.height / 2,
        0
      )
    );
  }

  toSENode() {
    return {
      ...super.toSENode(),
      p: this.p,
      execStr: this.execStr,
      next: this.exits[0]?.toNodeId || '',
    } as GameEventChildExec;
  }

  calculateHeight(ctx: CanvasRenderingContext2D) {
    const availableWidth = NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2;
    const allText = this.p + '\n\n' + this.execStr;
    const lines = breakTextIntoLines(
      allText,
      availableWidth,
      FONT_SIZE,
      FONT_FAMILY,
      ctx
    );
    const calculatedHeight = calculateHeightFromText(
      lines,
      FONT_SIZE,
      FONT_FAMILY,
      LINE_HEIGHT,
      LINE_SPACING,
      ctx
    );

    this.height =
      calculatedHeight + NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
    this.update();
  }

  update() {
    super.update();
    const connector = this.exits[0];
    if (connector) {
      connector.startX = this.x + this.width;
      connector.startY = this.y + this.height / 2;

      if (connector.toNodeId) {
        const childNode = this.editorState.editorNodes.find(
          (node) => node.id === connector.toNodeId
        );
        if (childNode) {
          const { x, y } = childNode.getEntrancePos();
          connector.endX = x;
          connector.endY = y;
        }
      }
    }
  }

  render(ctx: CanvasRenderingContext2D, scale: number, args: RenderNodeArgs) {
    super.render(ctx, scale, args);

    const p = this.p;
    const execStr = this.execStr;

    const availableWidth = NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2;
    // const allText = p + '\n\n' + execStr;
    const linesText = breakTextIntoLines(
      p,
      availableWidth,
      FONT_SIZE,
      FONT_FAMILY,
      ctx
    );

    const linesExecStr = breakTextIntoLines(
      execStr,
      availableWidth,
      FONT_SIZE,
      FONT_FAMILY,
      ctx
    );

    const nodeX = this.x * scale;
    const nodeY = this.y * scale;

    // title
    ctx.save();
    ctx.translate(nodeX, nodeY);
    ctx.scale(scale, scale);
    ctx.translate(PADDING + BORDER_WIDTH, PADDING + BORDER_WIDTH);
    drawText(
      this.id,
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

    for (let i = 0; i < linesText.length; i++) {
      ctx.save();
      ctx.translate(nodeX, nodeY);
      ctx.scale(scale, scale);
      ctx.translate(
        PADDING + BORDER_WIDTH,
        PADDING + BORDER_WIDTH + NODE_TITLE_HEIGHT
      );
      drawText(
        linesText[i],
        0,
        i * LINE_HEIGHT,
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

    for (let i = 0; i < linesExecStr.length; i++) {
      const yOffset = (i + linesText.length + 1) * LINE_HEIGHT;
      ctx.save();
      ctx.translate(nodeX, nodeY);
      ctx.scale(scale, scale);
      ctx.translate(
        PADDING + BORDER_WIDTH,
        PADDING + BORDER_WIDTH + NODE_TITLE_HEIGHT
      );
      drawText(
        linesExecStr[i],
        0,
        yOffset,
        {
          color: 'yellow',
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
  }
}
