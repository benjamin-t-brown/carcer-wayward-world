import { GameEventChildSwitch, SwitchCase } from '../../types/assets';
import { drawText } from '../../utils/draw';
import { EditorNode, RenderNodeArgs } from '../EditorNode';
import { Connector } from './Connector';
import { EditorStateSE } from '../seEditorState';

const NODE_COLOR = '#141F42';
const BORDER_COLOR = '#aaa';
const BORDER_HOVER_COLOR = '#fff';
const TEXT_COLOR = '#FFF';
const FONT_FAMILY = 'courier';
const NODE_WIDTH = 300;
const NODE_TITLE_HEIGHT = 20;
const BORDER_WIDTH = 2;
const FONT_SIZE = 12;
const PADDING = 6;
const LINE_HEIGHT = 24;

export class EditorNodeSwitch extends EditorNode {
  cases: SwitchCase[] = [];
  defaultNext: string = '';

  constructor(seNode: GameEventChildSwitch, editorState: EditorStateSE) {
    super(seNode, editorState);

    this.bgColor = NODE_COLOR;
    this.borderColor = BORDER_COLOR;
    this.borderColorHovered = BORDER_HOVER_COLOR;
    this.padding = PADDING;
    this.borderSize = BORDER_WIDTH;

    this.cases = seNode.cases.slice();
    this.defaultNext = seNode.defaultNext;
    this.exits.push(
      Connector.create(
        this.id,
        this.defaultNext,
        this.x + this.width,
        this.y + this.height / 2
      )
    );
    for (const c of this.cases) {
      this.exits.push(
        Connector.create(
          this.id,
          c.next,
          this.x + this.width,
          this.y + this.height / 2
        )
      );
    }
  }

  toSENode() {
    return {
      ...super.toSENode(),
      cases: this.cases,
      defaultNext: this.defaultNext,
    } as GameEventChildSwitch;
  }

  getMinHeight() {
    return NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
  }

  calculateHeight() {
    const totalCaseHeight = (this.cases.length + 1) * LINE_HEIGHT;
    this.height = Math.max(
      this.getMinHeight(),
      totalCaseHeight + this.getMinHeight()
    );
  }

  update() {
    super.update();
    const yOffset = NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
    for (let i = 0; i < this.exits.length; i++) {
      const conn = this.exits[i];
      conn.startX = this.x + this.width;
      conn.startY = this.y + yOffset + i * LINE_HEIGHT;

      if (conn.toNodeId) {
        const childNode = this.editorState.editorNodes.find(
          (node) => node.id === conn.toNodeId
        );
        if (childNode) {
          const { x, y } = childNode.getEntrancePos();
          conn.endX = x;
          conn.endY = y;
        }
      }
    }
  }

  render(ctx: CanvasRenderingContext2D, scale: number, args: RenderNodeArgs) {
    const nodeX = this.x * scale;
    const nodeY = this.y * scale;

    super.render(ctx, scale, args);

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
        font: 'arial',
        strokeColor: '',
        align: 'left',
        baseline: 'top',
      },
      ctx
    );
    ctx.restore();

    const caseTextOffset = 9;

    const textToRender = ['default', ...this.cases.map((c) => c.conditionStr)];
    for (const text of textToRender) {
      ctx.save();
      ctx.translate(nodeX, nodeY);
      ctx.scale(scale, scale);
      ctx.translate(PADDING + BORDER_WIDTH, caseTextOffset);
      drawText(
        text,
        0,
        0,
        {
          color: TEXT_COLOR,
          size: FONT_SIZE,
          font: FONT_FAMILY,
          strokeColor: '',
          align: 'left',
        },
        ctx
      );
      ctx.lineWidth = 1;
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.4)';
      ctx.beginPath();
      ctx.moveTo(0, caseTextOffset - 2);
      ctx.lineTo(
        NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2,
        caseTextOffset - 2
      );
      ctx.stroke();
      ctx.restore();
    }
  }
}
