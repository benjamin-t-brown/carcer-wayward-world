import { AudioInfo, GameEventChildExec } from '../../types/assets';
import { drawText } from '../../utils/draw';
import {
  breakTextIntoLines,
  calculateHeightFromText,
  truncateText,
} from '../nodeHelpers';
import { EditorNode, RenderNodeArgs } from './EditorNode';
import { Connector } from './Connector';
import { EditorStateSE } from '../seEditorState';

const NODE_COLOR = '#808080';
const BORDER_COLOR = '#aaa';
const BORDER_AUTO_ADVANCE_COLOR = '#99ffff';
const TEXT_COLOR = '#FFF';
const FONT_FAMILY = 'arial';
const NODE_WIDTH = 300;
const NODE_TITLE_HEIGHT = 20;
const BORDER_WIDTH = 2;
const FONT_SIZE = 12;
const PADDING = 6;
const LINE_SPACING = 4;
const LINE_HEIGHT = 14;
const P_EXEC_SPACING = 14;

export class EditorNodeExec extends EditorNode {
  p: string = '';
  execStr: string = '';

  pLines: string[] = [];
  execStrLines: string[] = [];
  pHeight = 0;
  execStrHeight = 0;

  audioInfo: AudioInfo | undefined;
  autoAdvance: boolean = false;

  constructor(seNode: GameEventChildExec, editorState: EditorStateSE) {
    super(seNode, editorState);

    this.bgColor = NODE_COLOR;
    this.borderColor = BORDER_COLOR;
    this.padding = PADDING;
    this.borderSize = BORDER_WIDTH;

    this.p = seNode.p ?? '';
    this.execStr = seNode.execStr ?? '';
    this.exits.push(
      Connector.create(
        this.id,
        seNode.next,
        this.x + this.width,
        this.y + this.height / 2,
        0
      )
    );
    this.autoAdvance = seNode.autoAdvance ?? true;

    this.audioInfo = structuredClone(seNode.audioInfo);
  }

  toSENode() {
    return {
      ...super.toSENode(),
      p: this.p,
      execStr: this.execStr,
      next: this.exits[0]?.toNodeId || '',
      audioInfo: this.audioInfo,
      autoAdvance: this.autoAdvance,
    } as GameEventChildExec;
  }

  build(ctx: CanvasRenderingContext2D) {
    const availableWidth = NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2;
    const pLines = breakTextIntoLines(
      this.p,
      availableWidth,
      FONT_SIZE,
      FONT_FAMILY,
      ctx
    );
    const execStrLines = breakTextIntoLines(
      this.execStr,
      999,
      FONT_SIZE,
      FONT_FAMILY,
      ctx
    );

    this.pLines = pLines;
    this.execStrLines = execStrLines;
    this.calculateHeight(ctx);
  }

  calculateHeight(ctx: CanvasRenderingContext2D) {
    const pHeight = calculateHeightFromText(
      this.pLines,
      FONT_SIZE,
      FONT_FAMILY,
      LINE_HEIGHT,
      LINE_SPACING,
      ctx
    );

    const execStrHeight = calculateHeightFromText(
      this.execStrLines,
      FONT_SIZE,
      FONT_FAMILY,
      LINE_HEIGHT,
      LINE_SPACING,
      ctx
    );

    this.pHeight = Boolean(this.p) ? pHeight : 0;
    this.execStrHeight = Boolean(this.execStr) ? execStrHeight : 0;
    const spacing = P_EXEC_SPACING;

    let height = NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
    height += this.pHeight;
    if (this.pHeight > 0 && this.execStrHeight > 0) {
      height += spacing;
    }
    height += this.execStrHeight;

    this.height = Math.max(height, 50);
    this.update(0);
  }

  update(dt: number) {
    super.update(dt);
    this.borderColor = this.autoAdvance ? BORDER_AUTO_ADVANCE_COLOR : BORDER_COLOR;

    const conn = this.exits[0];
    if (conn) {
      const startX = this.x + this.width;
      const startY = this.y + this.height / 2 + 7;

      if (conn.toNodeId) {
        const childNode = this.editorState.editorNodes.find(
          (node) => node.id === conn.toNodeId
        );
        if (childNode) {
          const { x, y } = childNode.getEntrancePos();
          const endX = x;
          const endY = y;
          conn.updatePosition(startX, startY, endX, endY);
        }
      } else {
        conn.updatePosition(startX, startY, 0, 0);
      }
    }
  }

  render(ctx: CanvasRenderingContext2D, scale: number, args: RenderNodeArgs) {
    this.prepareRender(ctx);
    super.render(ctx, scale, args);

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

    let yOffset = PADDING + BORDER_WIDTH + NODE_TITLE_HEIGHT;

    for (let i = 0; i < this.pLines.length; i++) {
      ctx.save();
      ctx.translate(nodeX, nodeY);
      ctx.scale(scale, scale);
      ctx.translate(PADDING + BORDER_WIDTH, yOffset + i * LINE_HEIGHT);
      drawText(
        this.pLines[i],
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

    if (this.pHeight) {
      yOffset += this.pHeight + 14;
    }

    for (let i = 0; i < this.execStrLines.length; i++) {
      const text = this.execStrLines[i];
      ctx.save();
      ctx.translate(nodeX, nodeY);
      ctx.scale(scale, scale);
      ctx.translate(PADDING + BORDER_WIDTH, yOffset + i * LINE_HEIGHT);
      drawText(
        truncateText(text, 36),
        0,
        0,
        {
          color: 'yellow',
          size: FONT_SIZE,
          font: 'courier',
          strokeColor: '',
          align: 'left',
          baseline: 'top',
        },
        ctx
      );
      ctx.restore();
    }

    this.finishRender(ctx);
  }
}
