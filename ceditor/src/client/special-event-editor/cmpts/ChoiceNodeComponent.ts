import {
  Choice,
  GameEventChildChoice,
  GameEventChildSwitch,
  SwitchCase,
} from '../../types/assets';
import { drawText } from '../../utils/draw';
import { EditorNode, RenderNodeArgs } from './EditorNode';
import { Connector } from './Connector';
import { EditorStateSE } from '../seEditorState';
import {
  breakTextIntoLines,
  calculateHeightFromText,
  truncateText,
} from '../nodeHelpers';

const NODE_COLOR = '#3978A8';
const BORDER_COLOR = '#aaa';
const BORDER_HOVER_COLOR = '#fff';
const TEXT_COLOR = '#FFF';
const FONT_FAMILY = 'arial';
const NODE_WIDTH = 300;
const NODE_TITLE_HEIGHT = 20;
const BORDER_WIDTH = 2;
const FONT_SIZE = 12;
const PADDING = 6;
const LINE_HEIGHT = 24;
const TEXT_LINE_HEIGHT = 14;
const TEXT_LINE_CHOICES_SPACING = 14;

export class EditorNodeChoice extends EditorNode {
  choices: Choice[] = [];
  text: string = '';

  textLines: string[] = [];
  textLinesHeight: number = 0;


  constructor(seNode: GameEventChildChoice, editorState: EditorStateSE) {
    super(seNode, editorState);

    this.bgColor = NODE_COLOR;
    this.borderColor = BORDER_COLOR;
    this.borderColorHovered = BORDER_HOVER_COLOR;
    this.padding = PADDING;
    this.borderSize = BORDER_WIDTH;

    this.text = seNode.text;
    this.choices = seNode.choices.slice();

    for (let i = 0; i < this.choices.length; i++) {
      const c = this.choices[i];
      this.exits.push(
        Connector.create(
          this.id,
          c.next,
          this.x + this.width,
          this.y + this.height / 2,
          i
        )
      );
    }
  }

  toSENode() {
    return {
      ...super.toSENode(),
      text: this.text,
      choices: this.exits.map((conn, i) => {
        const choice = this.choices[i];
        return {
          text: choice?.text ?? '',
          conditionStr: choice?.conditionStr ?? '',
          evalStr: choice?.evalStr ?? '',
          next: conn.toNodeId ?? '',
          prefixText: choice?.prefixText ?? '',
        };
      }),
    } as GameEventChildChoice;
  }

  getMinHeight() {
    return NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
  }

  buildFromChoices(choices: Choice[], ctx: CanvasRenderingContext2D) {
    this.choices = choices;
    this.exits = [];
    for (let i = 0; i < choices.length; i++) {
      const c = choices[i];
      this.exits.push(
        Connector.create(
          this.id,
          c.next,
          this.x + this.width,
          this.y + this.height / 2,
          i
        )
      );
    }
    this.textLines = breakTextIntoLines(
      this.text,
      NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2,
      FONT_SIZE,
      FONT_FAMILY,
      ctx
    );

    this.calculateHeight(ctx);
  }

  calculateHeight(ctx: CanvasRenderingContext2D) {
    const textHeight = calculateHeightFromText(
      this.textLines,
      FONT_SIZE,
      FONT_FAMILY,
      TEXT_LINE_HEIGHT,
      0,
      ctx
    );

    let height = this.getMinHeight();

    if (this.text) {
      height += textHeight;
      height += TEXT_LINE_CHOICES_SPACING;
    }

    const totalChoiceHeight = this.choices.length * LINE_HEIGHT;
    height += totalChoiceHeight;

    this.textLinesHeight = this.text
      ? textHeight + TEXT_LINE_CHOICES_SPACING
      : 0;
    this.height = height;
  }

  update(dt: number) {
    super.update(dt);
    const yOffset = this.getMinHeight() + this.textLinesHeight;
    for (let i = 0; i < this.exits.length; i++) {
      const conn = this.exits[i];
      const startX = this.x + this.width;
      const startY = this.y + yOffset + i * LINE_HEIGHT;

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
    const nodeX = this.x * scale;
    const nodeY = this.y * scale;

    this.prepareRender(ctx);

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

    for (let i = 0; i < this.textLines.length; i++) {
      const text = this.textLines[i];
      ctx.save();
      ctx.translate(nodeX, nodeY);
      ctx.scale(scale, scale);
      ctx.translate(
        PADDING + BORDER_WIDTH,
        this.getMinHeight() + caseTextOffset + i * TEXT_LINE_HEIGHT
      );
      drawText(
        text,
        0,
        0,
        {
          color: 'lightblue',
          size: FONT_SIZE,
          font: FONT_FAMILY,
          strokeColor: '',
          align: 'left',
        },
        ctx
      );
      ctx.restore();
    }

    for (let i = 0; i < this.choices.length; i++) {
      const choice = this.choices[i];
      const text = choice.conditionStr ? `!{CND} ${choice.text}` : choice.text;
      ctx.save();
      ctx.translate(nodeX, nodeY);
      ctx.scale(scale, scale);
      ctx.translate(
        PADDING + BORDER_WIDTH,
        this.getMinHeight() +
          caseTextOffset +
          i * LINE_HEIGHT +
          this.textLinesHeight
      );
      drawText(
        truncateText(text, 36),
        0,
        0,
        {
          color: choice.conditionStr ? '#aff' : TEXT_COLOR,
          size: FONT_SIZE,
          font: 'courier',
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

    this.finishRender(ctx);
  }
}
