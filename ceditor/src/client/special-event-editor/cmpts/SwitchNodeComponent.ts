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
        this.y + this.height / 2,
        0
      )
    );
    for (let i = 0; i < this.cases.length; i++) {
      const c = this.cases[i];
      this.exits.push(
        Connector.create(
          this.id,
          c.next,
          this.x + this.width,
          this.y + this.height / 2,
          i + 1
        )
      );
    }
  }

  toSENode() {
    return {
      ...super.toSENode(),
      cases: this.exits.slice(1).map((exit, i) => {
        const c = this.cases[i];
        return {
          conditionStr: c.conditionStr,
          next: exit.toNodeId || '',
        };
      }),
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

  buildFromCases(cases: SwitchCase[]) {
    this.cases = cases;
    this.exits = [];
    this.exits.push(
      Connector.create(
        this.id,
        this.defaultNext,
        this.x + this.width,
        this.y + this.height / 2,
        0
      )
    );
    for (let i = 0; i < cases.length; i++) {
      const c = cases[i];
      this.exits.push(
        Connector.create(
          this.id,
          c.next,
          this.x + this.width,
          this.y + this.height / 2,
          i + 1
        )
      );
    }
    this.calculateHeight();
  }

  addCase(c: SwitchCase) {
    this.cases.push(c);
    this.exits.push(
      Connector.create(
        this.id,
        c.next,
        this.x + this.width,
        this.y + this.height / 2,
        this.cases.length + 1
      )
    );
    this.calculateHeight();
  }
  removeCase(index: number) {
    if (index === 0) {
      console.error('Cannot remove default case');
      return;
    }
    this.cases.splice(index, 1);
    this.exits.splice(index, 1);
    for (let i = 0; i < this.exits.length; i++) {
      const conn = this.exits[i];
      conn.exitIndex = i + 1;
    }
    this.calculateHeight();
  }
  moveCase(currentIndex: number, direction: -1 | 1) {
    if (direction === -1) {
      if (currentIndex === 0) {
        console.error('Cannot move default case up');
        return;
      }
    }

    const newIndex = currentIndex + direction;
    const newCase = this.cases[newIndex];
    if (!newCase) {
      console.error('Cannot move case to invalid index');
      return;
    }
    this.cases[currentIndex] = newCase;
    this.cases[newIndex] = this.cases[currentIndex];
    this.exits[currentIndex] = this.exits[newIndex];

    for (let i = 0; i < this.exits.length; i++) {
      const conn = this.exits[i];
      conn.exitIndex = i + 1;
    }
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

    const caseTextOffset = 5;

    const textToRender = ['default', ...this.cases.map((c) => c.conditionStr)];
    for (let i = 0; i < textToRender.length; i++) {
      const text = textToRender[i];
      ctx.save();
      ctx.translate(nodeX, nodeY);
      ctx.scale(scale, scale);
      ctx.translate(
        PADDING + BORDER_WIDTH,
        this.getMinHeight() + caseTextOffset + i * LINE_HEIGHT
      );
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
      ctx.moveTo(0, caseTextOffset);
      ctx.lineTo(NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2, caseTextOffset);
      ctx.stroke();
      ctx.restore();
    }
  }
}
