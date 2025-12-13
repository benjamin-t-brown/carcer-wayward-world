import { GameEventChildEnd } from '../../types/assets';
import { drawText } from '../../utils/draw';
import { EditorNode, RenderNodeArgs } from './EditorNode';
import { EditorStateSE } from '../seEditorState';

const NODE_COLOR = '#772222'; // Red background
const BORDER_COLOR = '#c88';
const BORDER_HOVER_COLOR = '#faa';
const TEXT_COLOR = '#FFF';
const FONT_FAMILY = 'arial';
const NODE_WIDTH = 120;
const NODE_HEIGHT = 60;
const BORDER_WIDTH = 2;
const FONT_SIZE = 16;
const PADDING = 6;

export class EditorNodeEnd extends EditorNode {
  constructor(seNode: GameEventChildEnd, editorState: EditorStateSE) {
    super(seNode, editorState);

    this.bgColor = NODE_COLOR;
    this.borderColor = BORDER_COLOR;
    this.borderColorHovered = BORDER_HOVER_COLOR;
    this.padding = PADDING;
    this.borderSize = BORDER_WIDTH;
    this.width = NODE_WIDTH;
    this.height = NODE_HEIGHT;
  }

  toSENode() {
    return {
      ...super.toSENode(),
      next: '',
    } as GameEventChildEnd;
  }

  calculateHeight(_ctx: CanvasRenderingContext2D) {
    // Fixed height for end node
    this.height = NODE_HEIGHT;
    this.update(0);
  }

  update(dt: number) {
    super.update(dt);
  }

  render(ctx: CanvasRenderingContext2D, scale: number, args: RenderNodeArgs) {
    this.prepareRender(ctx);
    super.render(ctx, scale, args);

    const nodeX = this.x * scale;
    const nodeY = this.y * scale;

    // Draw "end" text centered in the node
    ctx.save();
    ctx.translate(nodeX, nodeY);
    ctx.scale(scale, scale);
    drawText(
      'end',
      this.width / 2,
      this.height / 2,
      {
        color: TEXT_COLOR,
        size: FONT_SIZE,
        font: FONT_FAMILY,
        strokeColor: '',
        align: 'center',
        baseline: 'middle',
      },
      ctx
    );
    ctx.restore();

    this.finishRender(ctx);
  }
}
